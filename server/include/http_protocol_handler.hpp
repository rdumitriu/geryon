/*
 * HTTPProtocolHandler.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */

#ifndef HTTPPROTOCOL_HANDLER_HPP_
#define HTTPPROTOCOL_HANDLER_HPP_

#include "http_executor.hpp"
#include "tcp_protocol_handler.hpp"
#include "http_server_types.hpp"
#include "http_request_parser.hpp"

namespace geryon { namespace server {

///
/// \brief The HttpProtocolHandler class
///
/// Deals with chunked transfers; once the request is read, it passes the request and response to the executor
class HttpProtocolHandler : public TCPProtocolHandler {
public:
    ///The constructor
    explicit HttpProtocolHandler(GMemoryPool * const _pMemoryPool,
                                 HttpExecutor * const executor,
                                 std::size_t maximalContentLength);
    ///Destructor
    virtual ~HttpProtocolHandler() {}

    /// \brief Called just before the protocol is used
    virtual void init(TCPConnection * _pConnection);

    ///
    /// \brief We read the bytes, this is where we process them.
    ///
    /// The current marker in the buffers shows where to start.
    ///
    /// \param currentBuffer the current buffer
    /// \param nBytes the number of bytes needing processing
    ///
    virtual void handleRead(GBufferHandler && currentBuffer, std::size_t nBytes);

private:

    void outputErrorString(const std::string & errorMsg);

    bool parseChunkedTESize(char c, geryon::HttpResponse::HttpStatusCode & statusCode);

    void sendStockAnswer(HttpResponse::HttpStatusCode http_code);

    HttpExecutor * pExecutor;

    geryon::server::HttpServerRequest request;
    geryon::server::HttpServerResponse response;

    HttpRequestParser parser; //only deals with the uninterrupted stream (no 100-continue)

    std::string chunkedDelimiterLine;
    std::size_t chunkSize;
    std::size_t totalChunkedSize;
    std::size_t chunkTransferredSz;
    std::size_t maximalContentLength;
    bool chunkedTransfer;
    unsigned int chunkedState;
};

} }/* namespace */

#endif
