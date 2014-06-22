
#ifndef GERYON_TCPPROTOCOL_HPP_
#define GERYON_TCPPROTOCOL_HPP_

#include <string>

namespace geryon { namespace server {

class TCPProtocolHandler;

class TCPProtocol {
public:
    TCPProtocol(const std::string & _name) : nm(_name) {}
    virtual ~TCPProtocol() {}

    inline std::string name() { return nm; }

    virtual std::shared_ptr<TCPProtocolHandler> createHandler() = 0;
private:
    std::string nm;
};

} }

#endif
