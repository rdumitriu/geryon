///
/// http_protocol_handler.hpp
///
///  Created on: Aug 25, 2011
///      Author: rdumitriu
///

#ifndef GERYON_HTTPPROTOCOL_HANDLER_HPP_
#define GERYON_HTTPPROTOCOL_HANDLER_HPP_

#include "http_executor.hpp"
#include "tcp_protocol_handler.hpp"
#include "http_server_types.hpp"
#include "http_request_parser_base.hpp"

namespace geryon { namespace server {

///
/// \brief The HttpProtocolHandler class
///
/// Deals with chunked transfers; once the request is read, it passes the request and response to the executor
class HttpProtocolHandler : public TCPProtocolHandler {
public:
    ///The constructor
    explicit HttpProtocolHandler(GMemoryPool * const _pMemoryPool,
                                 HttpExecutor & executor,
                                 std::size_t maximalContentLength);
    ///Destructor
    virtual ~HttpProtocolHandler() {}

    /// \brief Called just before the protocol is used
    virtual void init(std::shared_ptr<TCPConnection> _pConnection);

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

    void sendStockAnswer(HttpResponse::HttpStatusCode http_code, const std::string & msg);

    bool switchParsersToTE(HttpResponse::HttpStatusCode & http_code);

    HttpExecutor & executor;

    geryon::server::HttpServerRequest request;
    geryon::server::HttpServerResponse response;

    std::shared_ptr<AbstractHttpRequestParserBase> pParser;

    std::size_t totalBytesProcessed;
    std::size_t maxContentLength;
};

} }/* namespace */

#endif
