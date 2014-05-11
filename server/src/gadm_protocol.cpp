///
/// \file gadm_protocol.cpp
///
///  Created on: May 11, 2014
///      Author: rdumitriu at gmail.com

#include "gadm_protocol.hpp"
#include "tcp_protocol_handler.hpp"
#include "server_global_structs.hpp"

#include "string_utils.hpp"
#include "log.hpp"

namespace geryon { namespace server {

namespace detail {

class GAdmTCPProtocolHandler : public TCPProtocolHandler {
public:
    GAdmTCPProtocolHandler(GMemoryPool & _rMemoryPool) : TCPProtocolHandler(_rMemoryPool) {}
    virtual ~GAdmTCPProtocolHandler() {}
    virtual void init(TCPConnection & _rConnection);
    virtual void handleRead(GBufferHandler && currentBuffer, std::size_t nBytes);
    virtual void done();
private:
    std::string command;
};

void GAdmTCPProtocolHandler::init(TCPConnection & _rConnection) {
    TCPProtocolHandler::init(_rConnection);

    LOG(geryon::util::Log::DEBUG) << "Connected to administrative interface";

//    GBufferHandler writeBuff(&getMemoryPool());
//    std::string msg("Connected to geryon server administrative interface.\n");
//    std::strncpy(writeBuff.get().buffer(), msg.c_str(), writeBuff.get().size());
//    writeBuff.get().setMarker(msg.length());
//    requestWrite(std::move(writeBuff));

    GBufferHandler readBuffer(&getMemoryPool());
    requestRead(std::move(readBuffer));
}

void GAdmTCPProtocolHandler::handleRead(GBufferHandler && currentBuffer, std::size_t nBytes) {
    LOG(geryon::util::Log::DEBUG) << "Read handler called with " << nBytes << " bytes read.";
    std::string str;
    for(unsigned int i = 0; i < nBytes + currentBuffer.get().marker(); ++i) {
        str.push_back(currentBuffer.get().buffer()[i]);
    }
    LOG(geryon::util::Log::DEBUG) << "Buffer content is >>" << str << "<<";

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
    geryon::util::trim(command);

    LOG(geryon::util::Log::DEBUG) << "Command is >>" << command << "<<";
    if("close" == command) {
        LOG(geryon::util::Log::DEBUG) << "Disconnected from administrative interface (cmd)";
        requestClose();
    } else if("help" == command) {
        LOG(geryon::util::Log::DEBUG) << "Gadm :: Help";
        GBufferHandler writeBuff(&getMemoryPool());
        std::string msg("Help requested.\n");
        std::strncpy(writeBuff.get().buffer(), msg.c_str(), writeBuff.get().size());
        writeBuff.get().setMarker(msg.length());
        requestWrite(std::move(writeBuff));
        command.clear();
    }
    if(hascr) {
        command.clear();
    }

    LOG(geryon::util::Log::DEBUG) << "Protocol will request read";
    GBufferHandler readBuffer(&getMemoryPool());
    requestRead(std::move(readBuffer));
    LOG(geryon::util::Log::DEBUG) << "Protocol readHandler done";
}

void GAdmTCPProtocolHandler::done() {
    LOG(geryon::util::Log::DEBUG) << "Disconnected from administrative interface (done)";
}

} /*namespace detail */

std::shared_ptr<TCPProtocolHandler> GAdmProtocol::createHandler() {
    return std::make_shared<detail::GAdmTCPProtocolHandler>(*ServerGlobalStucts::getMemoryPool());
}


} }
