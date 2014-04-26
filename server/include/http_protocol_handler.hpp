/*
 * HTTPProtocolHandler.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */

#ifndef HTTPPROTOCOL_HANDLER_HPP_
#define HTTPPROTOCOL_HANDLER_HPP_

#include "http_types.hpp"
#include "tcp_protocol_handler.hpp"

namespace geryon { namespace server {

class HttpProtocolHandler : public TCPProtocolHandler {
public:
    ///The constructor
    explicit HttpProtocolHandler() : TCPProtocolHandler(true) {}
    ///Destructor
    virtual ~HttpProtocolHandler() {}

    ///Called just before the protocol is used
    virtual void init() {}

    ///Called by the connection when we read the bytes
    virtual void read(std::size_t nBytes) = 0;

    ///The read buffer
    virtual boost::asio::mutable_buffers_1 readBuffer() = 0;

    ///Called by the connection when we wrote the bytes
    virtual void write(std::size_t nBytes) = 0;

    ///The write buffer
    virtual boost::asio::mutable_buffers_1 writeBuffer() = 0;

    ///called just before exit time
    virtual void done() {}

private:
    std::string extractFirstPathSegment(const std::string & path);

    void outputErrorString(const std::string & errorMsg);

    bool parseChunkedTESize(char c, geryon::HttpResponse::HttpStatusCode & statusCode);

    void send100Continue();

//    TCPServerConfig * m_pServerConfig;
//    agrade::yahsrv::HTTPRequest m_Request;
//    agrade::yahsrv::HTTPReply m_Reply;
//    HTTPRequestParser m_parser;
//    agrade::yahsrv::Servlet * m_pServlet;

    std::string m_chunkedDelimiterLine;
    std::size_t m_chunkSize;
    std::size_t m_totalChunkedSize;
    std::size_t m_chunkTransferredSz;
    bool m_chunkedTransfer;
    unsigned int m_chunkedState;
};

} }/* namespace */

#endif
