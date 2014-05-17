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


HttpProtocolHandler::HttpProtocolHandler(GMemoryPool * _pMemoryPool, std::size_t maximalContentLength)
                        : TCPProtocolHandler(_pMemoryPool),
                      request(),
                      response(),
                      parser(maximalContentLength),
                      chunkedDelimiterLine(""),
                      chunkSize(0L),
                      totalChunkedSize(0L),
                      chunkTransferredSz(0L),
                      chunkedTransfer(false),
                      chunkedState(0) {
}

void HttpProtocolHandler::init(TCPConnection * _pConnection) {
    TCPProtocolHandler::init(_pConnection);
    parser.init(&request);
    try {
        GBufferHandler readBuff(getMemoryPool());
        requestRead(std::move(readBuff));
    } catch( ... ) {
        requestClose();
    }
}

void HttpProtocolHandler::handleRead(GBufferHandler && currentBuffer, std::size_t nBytes) {
    try {
        bool done = false;
        geryon::HttpResponse::HttpStatusCode statusCode = geryon::HttpResponse::SC_OK;

        GBuffer buff = currentBuffer.get();
        for(unsigned int i = 0; i < nBytes && !done; ++i) {
            char c = buff.buffer()[buff.marker() + i];

            if(chunkedState) {
                done = parseChunkedTESize(c, statusCode);
                if(!chunkedState && !done) {
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
                            //:::TODO:: mark error
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
            } else {
                //::TODO:: proper dispatch
                sendStockAnswer(geryon::HttpResponse::SC_NOT_IMPLEMENTED);
            }
            //::TODO:: don't close, maybe use keepalive
            requestClose();
        } else {
            //read a bit more
            GBufferHandler readBuff(getMemoryPool());
            requestRead(std::move(readBuff));
        }
    } catch( ... ) {
        LOG(geryon::util::Log::ERROR) << "Exception while serving resource:"
                                      << request.getURI() << ".";
        try {
            sendStockAnswer(geryon::HttpResponse::SC_INTERNAL_SERVER_ERROR);
            requestClose();
        } catch( ... ) {} // just make sure.
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
