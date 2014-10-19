/**
 * \file http_protocol_handler.cpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */

#include <cstring>
#include <sstream>

#include "http_protocol_handler.hpp"
#include "http_request_parser.hpp"
#include "http_request_parser_te.hpp"

#include "log.hpp"

namespace geryon { namespace server {


HttpProtocolHandler::HttpProtocolHandler(GMemoryPool * _pMemoryPool, HttpExecutor & _executor, std::size_t maximalContentLength)
                        : TCPProtocolHandler(_pMemoryPool),
                      executor(_executor),
                      request(),
                      response(this),
                      totalBytesProcessed(0L),
                      maxContentLength(maximalContentLength) {
    LOG(geryon::util::Log::DEBUG) << "Created protocol handler";
}

void HttpProtocolHandler::init(std::shared_ptr<TCPConnection> _pConnection) {
    LOG(geryon::util::Log::DEBUG) << "About to init the protocol handler";
    TCPProtocolHandler::init(_pConnection);
    request.setConnection(_pConnection);
    pParser = std::make_shared<HttpRequestParser>(maxContentLength, totalBytesProcessed);
    pParser->init(&request);
    try {
        GBufferHandler readBuff(getMemoryPool());
        requestRead(std::move(readBuff));
        LOG(geryon::util::Log::DEBUG) << "Initialized protocol handler for connection";
    } catch( ... ) {
        requestClose();
    }
}

///
/// \brief HttpProtocolHandler::handleRead
/// \param currentBuffer the buffer to handle
/// \param nBytes the relevant bytes (read)
///
/// Note: the implementation makes effectively from the http_parsers a state machine
/// into another state machine. Care should be taken because meta-commands of the chunked transfer results in gaps
/// that must be introduced into the input stream.
///
void HttpProtocolHandler::handleRead(GBufferHandler && currentBuffer, std::size_t nBytes) {

    try {
        bool done = false;
        geryon::HttpResponse::HttpStatusCode statusCode = geryon::HttpResponse::SC_OK;
        LOG(geryon::util::Log::DEBUG) << "Request build - handle read:" << nBytes;

        GBuffer & buff = currentBuffer.get();
        // process the buffer, char by char
        unsigned int i = 0;
        for(; i < nBytes && !done; ++i) {
            ++totalBytesProcessed;
            char c = buff.buffer()[buff.marker() + i];
            LOG(geryon::util::Log::DEBUG) << "Process [" << totalBytesProcessed << "] char =" << c;
            AbstractHttpRequestParserBase::ParserAction action = pParser->consume(c, statusCode);
            switch(action) {
                case AbstractHttpRequestParserBase::PA_DONE:
                    done = true;
                    break;
                case AbstractHttpRequestParserBase::PA_CHECK_HEADERS:
                    done = switchParsersToTE(statusCode);
                    acceptRequest();
                    break;
                case AbstractHttpRequestParserBase::PA_CONTINUEACTION:
                    acceptRequest();
                    break;
                case AbstractHttpRequestParserBase::PA_CONTINUE:
                default:
                    break;
            }
        }
        //we're done parsing the current chars
        // in any case, advance the marker now we processed the bytes
        buff.advanceMarker(nBytes);

        if(done) {
            if(statusCode != geryon::HttpResponse::SC_OK) {
                sendStockAnswer(statusCode, "Request cannot be processed, check log");
                requestClose();
                return;
            } else {
//                LOG(geryon::util::Log::DEBUG) << "Start processing request i= " << i << " nBytes was =" << nBytes;
//                if(i != nBytes) {
//                    sendStockAnswer(geryon::HttpResponse::SC_BAD_REQUEST, "Incompletely parsed request");
//                    requestClose();
//                    return;
//                }
                LOG(geryon::util::Log::DEBUG) << "Request completed.";
                //1: add last buffer
                request.buffers.push_back(std::move(currentBuffer));
                //2: now, do the dispatch
                LOG(geryon::util::Log::DEBUG) << "Request about to be dispatched.";
                executor.execute(request, response);
            }
        } else {
            //read a bit more
            //::TODO:: at this stage, we should know in some cases the amount of bytes to read, so we should be able
            //::TODO:: to request bigger or smaller buffers. In any case, we'll reuse the buffer if we have 2k left
            LOG(geryon::util::Log::DEBUG) << "Request not completed, reading again (read :" << nBytes << " bytes)";
            if(buff.size() - buff.marker() > 2048) {
                //reuse the buffer
                requestRead(std::move(currentBuffer));
            } else {
                //push the current buffer in request
                request.buffers.push_back(std::move(currentBuffer));
                //allocate a new buffer and read again
                GBufferHandler readBuff(getMemoryPool());
                requestRead(std::move(readBuff));
            }
        }
    } catch(geryon::HttpException & e) {
        LOG(geryon::util::Log::ERROR) << "Exception while serving resource:"
                                      << request.getURI() << ". Error was:" << e.what();
        try {
            sendStockAnswer(geryon::HttpResponse::SC_INTERNAL_SERVER_ERROR, e.what());
        } catch( ... ) {}
        requestClose();
    } catch( ... ) {
        LOG(geryon::util::Log::ERROR) << "Exception while serving resource:"
                                      << request.getURI() << ".";
        try {
            sendStockAnswer(geryon::HttpResponse::SC_INTERNAL_SERVER_ERROR, "Uncaught exception, check logs");
        } catch( ... ) {} // just make sure.
        requestClose();
    }
}



void HttpProtocolHandler::sendStockAnswer(HttpResponse::HttpStatusCode http_code, const std::string & msg) {
    //::TODO:: instead of these, real stock answers!
    //::TODO:: buffers should be already allocated and shared, such as I will avoid any error
    GBufferHandler writeBuff(getMemoryPool());

    std::ostringstream stream;
    stream << getHttpStatusMessage(http_code);
    if(geryon::HttpResponse::SC_OK != http_code && geryon::HttpResponse::SC_CONTINUE != http_code) {
        stream << "Content-Type: text/plain\r\n";
        stream << "Connection: close\r\n\r\n\r\nError! Response produced:";
        stream << geryon::getHttpStatusMessage(http_code);
        stream << "Message:" << msg;
    }

    std::string ref = stream.str();
    std::strncpy(writeBuff.get().buffer(), ref.c_str(), writeBuff.get().size());
    writeBuff.get().setMarker(ref.length()); //length of the string
    LOG(geryon::util::Log::DEBUG) << "Sending stock answer:\n" << ref;
    requestWrite(std::move(writeBuff));
}

bool HttpProtocolHandler::switchParsersToTE(HttpResponse::HttpStatusCode & http_code) {
    //the order of TE header tells us how we should instantiate headers
    std::vector<std::string> tehdrs = request.getHeaderValues("Transfer-Encoding");
    bool chunkedFound = false;
    for(std::string & hdrv : tehdrs) {
        if(hdrv == "chunked") {
            if(!chunkedFound) { //ignore multiple TE headers values
                std::shared_ptr<AbstractHttpRequestParserBase> nprs;
                nprs = std::make_shared<HttpRequestParserChunkedTE>(pParser, maxContentLength, totalBytesProcessed);
                nprs->init(&request);
                pParser = nprs;
                chunkedFound = true;
            }
        } else if(hdrv == "") {
            //nothing to do.
        } else {
            http_code = geryon::HttpResponse::SC_NOT_IMPLEMENTED;
            return true;
        }
    }
    return false;
}

void HttpProtocolHandler::acceptRequest() {
    if(request.hasExpectHeader()) {
        sendStockAnswer(HttpResponse::SC_CONTINUE);
    }
}

} } /*namespace*/
