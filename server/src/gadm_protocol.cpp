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

    void writeString(const std::string & str);
private:
    std::string command;
    void writeString();
};

void GAdmTCPProtocolHandler::writeString(const std::string & msg) {
    GBufferHandler writeBuff(&getMemoryPool());
    std::strncpy(writeBuff.get().buffer(), msg.c_str(), writeBuff.get().size());
    writeBuff.get().setMarker(msg.length());
    requestWrite(std::move(writeBuff));
}

void GAdmTCPProtocolHandler::init(TCPConnection & _rConnection) {
    TCPProtocolHandler::init(_rConnection);

    LOG(geryon::util::Log::DEBUG) << "Connected to administrative interface (init)";

    writeString("Connected to GERYON. Type command and press enter.\n");
    writeString(">");
    GBufferHandler readBuffer(&getMemoryPool());
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
        geryon::util::trim(command);

        if("close" == command) {
            requestClose();
        } else if("help" == command) {
            writeString("Help:\n");
            writeString("\t close - closes this connection\n");
            writeString("\t help  - this message\n\n");
        } else {
            writeString("?Well? Help is available by typing 'help'\n");
        }
        command.clear();
        writeString(">");
    }

    GBufferHandler readBuffer(&getMemoryPool());
    requestRead(std::move(readBuffer));
}

void GAdmTCPProtocolHandler::done() {
    LOG(geryon::util::Log::DEBUG) << "Disconnected from administrative interface (done)";
}

} /*namespace detail */

std::shared_ptr<TCPProtocolHandler> GAdmProtocol::createHandler() {
    return std::make_shared<detail::GAdmTCPProtocolHandler>(*ServerGlobalStucts::getMemoryPool());
}


} }
