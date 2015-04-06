///
/// \file gadm_protocol.cpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#include <sstream>

#include "gadm_protocol.hpp"
#include "tcp_protocol_handler.hpp"
#include "server_global_structs.hpp"

#include "string_utils.hpp"
#include "log.hpp"

namespace geryon { namespace server {

namespace detail {

class GAdmTCPProtocolHandler : public TCPProtocolHandler {
public:
    GAdmTCPProtocolHandler(GMemoryPool * const _pMemoryPool) : TCPProtocolHandler(_pMemoryPool) {}
    virtual ~GAdmTCPProtocolHandler() {}
    virtual void init(std::shared_ptr<TCPConnection> _pConnection);
    virtual void handleRead(GBufferHandler && currentBuffer, std::size_t nBytes);
    virtual void done();

private:
    bool handleCommand(const std::string & command);

    void writeHelp();
    void showBuffers();
    void showApplications();
    void showApplicationSessions(const std::string & command);
    void showApplicationSessions(std::shared_ptr<ServerApplication> app);
    void showSQLPools();

    void writeString(const std::string & str);

    std::string command;
    void writeString();
};

void GAdmTCPProtocolHandler::writeString(const std::string & msg) {
    GBufferHandler writeBuff(getMemoryPool());
    std::strncpy(writeBuff.get().buffer(), msg.c_str(), writeBuff.get().size());
    writeBuff.get().setMarker(msg.length());
    requestWrite(std::move(writeBuff));
}

void GAdmTCPProtocolHandler::init(std::shared_ptr<TCPConnection> _pConnection) {
    TCPProtocolHandler::init(_pConnection);

    LOG(geryon::util::Log::DEBUG) << "Connected to administrative interface (init)";

    writeString("Connected to GERYON. Type command and press enter.\n>");
    GBufferHandler readBuffer(getMemoryPool());
    requestRead(std::move(readBuffer));
}

void GAdmTCPProtocolHandler::handleRead(GBufferHandler && currentBuffer, std::size_t nBytes) {
    bool hascr = false;
    GBuffer buff = currentBuffer.get();
    for(unsigned int i = 0; i < nBytes && !hascr; ++i) {
        char c = buff.buffer()[buff.marker() + i];
        if(c != '\n') {
            command.push_back(c);
        } else {
            hascr = true;
        }
    }

    if(hascr) {
        if(!handleCommand(command)) {
            writeString("?Well? Help is available by typing 'help'\n");
        }
        command.clear();
        writeString(">");
    }

    GBufferHandler readBuffer(getMemoryPool());
    requestRead(std::move(readBuffer));
}

bool GAdmTCPProtocolHandler::handleCommand(const std::string & cmd) {
    std::string command = geryon::util::trimAndCopy(cmd);

    if("close" == command) {
        requestClose();
        return true;
    } else if("help" == command) {
        writeHelp();
        return true;
    } else if("buff" == command) {
        showBuffers();
        return true;
    } else if("apps" == command) {
        showApplications();
        return true;
    } else if(geryon::util::startsWith(command, "sess ")) {
        showApplicationSessions(command);
        return true;
    } else if("sqlpools" == command) {
        showSQLPools();
        return true;
    }
    return false;
}

void GAdmTCPProtocolHandler::writeHelp() {
    writeString("Help:\n \
\t close - closes this connection\n \
\t buff - shows buffers used for I/O\n \
\t apps - shows applications\n \
\t sess <appkey> - shows sessions for application\n \
\t sqlpools - shows sql pools\n \
\t help  - this message\n\n");
}

void GAdmTCPProtocolHandler::showBuffers() {
    std::shared_ptr<GMemoryPool> mp = ServerGlobalStructs::getMemoryPool();

    std::ostringstream out;
    out << "Memory pool [ " << mp->getBufferSizeK() << "K ] allocated:"
                            << mp->getAllocatedSizeK() << "K, max size :"
                            << mp->getMaxSizeK() << "K\n";
    writeString(out.str());
}

void GAdmTCPProtocolHandler::showApplications() {
    std::vector<std::shared_ptr<ServerApplication>> apps = ServerGlobalStructs::getApplications();
    for(auto & i : apps) {
        std::shared_ptr<ServerApplication> app = i;
        std::string status;
        switch(app->getStatus()) {
            case ApplicationModule::INIT:
                status = "INIT";
                break;
            case ApplicationModule::STARTED:
                status = "STARTED";
                break;
            case ApplicationModule::STOPPED:
            default:
                status = "STOPPED";
                break;
        }

        std::ostringstream out;
        out << "Application " << app->getKey() << "\tMounted on: "
                              << app->getPath() << "\tStatus:"
                              << status << "\n";
        writeString(out.str());
    }
}

void GAdmTCPProtocolHandler::showApplicationSessions(const std::string &command) {
    if(command.length() <= 5) {
        writeString("Need application key.");
        return;
    }
    std::string appkey = geryon::util::trimAndCopy(command.substr(5));
    if(appkey == "") {
        writeString("We really need the application key.");
        return;
    }
    std::vector<std::shared_ptr<ServerApplication>> apps = ServerGlobalStructs::getApplications();
    for(auto & i : apps) {
        std::shared_ptr<ServerApplication> app = i;
        if(app->getKey() == appkey) {
            showApplicationSessions(app);
            return;
        }
    }
    writeString("No such application key:" + appkey);
}

void GAdmTCPProtocolHandler::showApplicationSessions(std::shared_ptr<ServerApplication> app) {
    std::vector<detail::SessionStats> stats = app->getStats();
    int i = 0;
    std::size_t overallCount = 0;
    std::size_t overallSize = 0;
    for(auto ss : stats) {
        overallCount += ss.count;
        overallSize += ss.totalSize;
        std::ostringstream out;
        out << "Partition [" << i << "] Count: " << ss.count << "\tSize: " << ss.totalSize << "\n";
        writeString(out.str());
    }
    std::ostringstream out;
    out << "===\n\nTotal count: " << overallCount << "\tTotal size: " << overallSize << "\n";
    if(overallCount > 0) {
        out << "Average session size: " << (overallSize / overallCount) << " bytes \n";
    }
    writeString(out.str());
}

void GAdmTCPProtocolHandler::showSQLPools() {
#ifdef G_HAS_PQXX
    std::map<std::string, geryon::sql::postgres::PostgresConnectionPoolPtr> sqlp = ServerGlobalStructs::getPostgresPools();
    for(auto & i : sqlp) {
        geryon::sql::postgres::PostgresConnectionPoolPtr sp = i.second;
        std::ostringstream out;
        out << i.first << ", type Postgres SQL [Min: " << sp->minSize() << " Max: " << sp->maxSize()
            << "] ready : " << sp->currentSize() << ", in use right now: " << sp->inUseSize() << "\n";
        writeString(out.str());
    }
#else
    writeString("Server has no SQL support.");
#endif
}

void GAdmTCPProtocolHandler::done() {
    LOG(geryon::util::Log::DEBUG) << "Disconnected from administrative interface (done)";
}

} /*namespace detail */

std::shared_ptr<TCPProtocolHandler> GAdmProtocol::createHandler() {
    return std::make_shared<detail::GAdmTCPProtocolHandler>(ServerGlobalStructs::getMemoryPool().get());
}


} }
