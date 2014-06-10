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

#include "log.hpp"

namespace geryon { namespace server {


HttpProtocolHandler::HttpProtocolHandler(GMemoryPool * _pMemoryPool, HttpExecutor & _executor, std::size_t maximalContentLength)
                        : TCPProtocolHandler(_pMemoryPool),
                      executor(_executor),
                      request(),
                      response(this),
                      parser(maximalContentLength),
                      chunkedDelimiterLine(""),
                      chunkSize(0L),
                      totalChunkedSize(0L),
                      chunkTransferredSz(0L),
                      chunkedTransfer(false),
                      chunkedState(0) {
    LOG(geryon::util::Log::DEBUG) << "Created protocol handler";
}

void HttpProtocolHandler::init(std::shared_ptr<TCPConnection> _pConnection) {
    LOG(geryon::util::Log::DEBUG) << "About to init the protocol handler";
    TCPProtocolHandler::init(_pConnection);
    request.setConnection(_pConnection);
    parser.init(&request);
    try {
        GBufferHandler readBuff(getMemoryPool());
        requestRead(std::move(readBuff));
        LOG(geryon::util::Log::DEBUG) << "Initialized protocol handler for connection";
    } catch( ... ) {
        requestClose();
    }
}

void HttpProtocolHandler::handleRead(GBufferHandler && currentBuffer, std::size_t nBytes) {
    try {
        bool done = false;
        geryon::HttpResponse::HttpStatusCode statusCode = geryon::HttpResponse::SC_OK;
        LOG(geryon::util::Log::DEBUG) << "Request build - handle read:" << nBytes;

        GBuffer & buff = currentBuffer.get();
        for(unsigned int i = 0; i < nBytes && !done; ++i) {
            char c = buff.buffer()[buff.marker() + i];

            if(chunkedState) {
                done = parseChunkedTESize(c, statusCode);
                if(!chunkedState && !done) { //chunkedState might be modified in here!
                    sendStockAnswer(geryon::HttpResponse::SC_CONTINUE);
                }
            } else {
                done = parser.consume(c, statusCode);
                if(!done) {
                    if(statusCode == geryon::HttpResponse::SC_CONTINUE) {
                        chunkTransferredSz = 0L;
                        sendStockAnswer(geryon::HttpResponse::SC_CONTINUE);
                        statusCode = geryon::HttpResponse::SC_OK;
                        if("" == request.getHeaderValue("Transfer-Encoding")) {
                            //not really a chunked transfer
                            chunkedTransfer = false;
                            chunkedState = 0;
                            sendStockAnswer(geryon::HttpResponse::SC_NOT_IMPLEMENTED);
                            requestClose();
                            return;
                        } else {
                            //a trully one, parser only accept 'chunked' at this stage
                            chunkedTransfer = true;
                            chunkedState = 1;
                        }
                    } else if(chunkedTransfer) {
                        chunkTransferredSz++;
                        if(chunkSize > 0 && chunkTransferredSz == chunkSize) {
                            chunkedState = 1;
                            chunkTransferredSz = 0L;
                        }
                    }
                }
            }
        }
        if(done) {
            if(statusCode != geryon::HttpResponse::SC_OK) {
                sendStockAnswer(statusCode);
                requestClose();
            } else {
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
            if(buff.size() - buff.marker() - nBytes > 2048) {
                //reuse the buffer
                buff.advanceMarker(nBytes);
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
            sendStockAnswer(geryon::HttpResponse::SC_INTERNAL_SERVER_ERROR);
            requestClose();
        } catch( ... ) {
            requestClose();
        }
    } catch( ... ) {
        LOG(geryon::util::Log::ERROR) << "Exception while serving resource:"
                                      << request.getURI() << ".";
        try {
            sendStockAnswer(geryon::HttpResponse::SC_INTERNAL_SERVER_ERROR);
            requestClose();
        } catch( ... ) {
            requestClose();
        } // just make sure.
    }
}

void HttpProtocolHandler::sendStockAnswer(HttpResponse::HttpStatusCode http_code) {
    //::TODO:: instead of these, real stock answers!
    //::TODO:: buffers should be already allocated and shared, such as I will avoid any error
    GBufferHandler writeBuff(getMemoryPool());
    std::string ref = geryon::getHttpStatusMessage(http_code);
    std::strncpy(writeBuff.get().buffer(), ref.c_str(), writeBuff.get().size());
    writeBuff.get().setMarker(ref.length()); //length of the header
    requestWrite(std::move(writeBuff));
}

bool HttpProtocolHandler::parseChunkedTESize(char c, geryon::HttpResponse::HttpStatusCode & statusCode) {
    //chunked transfer encoding, ignore first \r\n
    if(c == '\n' && chunkedDelimiterLine.size()) {
        std::istringstream sdelta(chunkedDelimiterLine);
        sdelta >> std::hex >> chunkSize;
        if(chunkSize != 0) {
            totalChunkedSize += chunkSize;
            //revalidate the maximal content length
            if(totalChunkedSize > parser.maximalContentLenght()) {
                LOG(geryon::util::Log::ERROR) << "Chunked transfer. Request too large; read so far:" << totalChunkedSize;
                statusCode = geryon::HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
                return true;
            }
        } else {
            request.contentLength = totalChunkedSize;
            chunkedState = 0;
            return true; //force end of parsing
        }
        //and clear this status back:
        chunkedState = 0;
        chunkedDelimiterLine.clear();
        return false;
    }
    if(c != '\r' && c != '\n') {
        chunkedDelimiterLine.push_back(c);
    }
    return false;
}

} } /*namespace*/
