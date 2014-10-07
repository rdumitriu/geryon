///
/// \file http_request_parser_te.cpp
///
///  Created on: Sep 29, 2014
///      Author: rdumitriu at gmail.com
///

#include "http_request_parser_te.hpp"

namespace geryon { namespace server {

HttpRequestParserChunkedTE::HttpRequestParserChunkedTE(std::shared_ptr<AbstractHttpRequestParserBase> _pRealParser,
                                                       std::size_t maximalContentLenght, std::size_t & absoluteIndex)
                                        : AbstractHttpRequestParserBase(maximalContentLenght, absoluteIndex),
                                          pRealParser(_pRealParser),
                                          chunkSize(0),
                                          chunkTransferredSz(0),
                                          chunkedState(1),
                                          gapStart(0) {
}

HttpRequestParserChunkedTE::~HttpRequestParserChunkedTE() {
}

AbstractHttpRequestParserBase::ParserAction
HttpRequestParserChunkedTE::consume(char c, geryon::HttpResponse::HttpStatusCode & error) {
    if(AbstractHttpRequestParserBase::ParserAction::PA_DONE == AbstractHttpRequestParserBase::consume(c, error)) {
        return AbstractHttpRequestParserBase::PA_DONE;
    }
    switch(chunkedState) {
        case 1:
            return parseChunkedTESize(c, error);
        case 0:
        default:
            return pRealParser->consume(c, error);
    }
}

AbstractHttpRequestParserBase::ParserAction
HttpRequestParserChunkedTE::parseChunkedTESize(char c, geryon::HttpResponse::HttpStatusCode & error) {
    if(gapStart == 0) {
        gapStart = getAbsoluteIndex();
    }
    //chunked transfer encoding, ignore first \r\n
    if(c == '\n' && chunkedDelimiterLine.size()) {
        std::istringstream sdelta(chunkedDelimiterLine);
        sdelta >> std::hex >> chunkSize;
        pRequest->contentLength += chunkSize;
        if(pRequest->contentLength > maximalContentLenght()) {
            error = geryon::HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
            return AbstractHttpRequestParserBase::ParserAction::PA_DONE;
        }
        //and clear this status back:
        chunkedState = 0;
        chunkedDelimiterLine.clear();
        pRequest->addInputStreamGap(gapStart, getAbsoluteIndex());
        gapStart = 0;
        return chunkSize == 0 ? AbstractHttpRequestParserBase::ParserAction::PA_DONE \
                              : AbstractHttpRequestParserBase::ParserAction::PA_CONTINUEACTION;
    } else if(c != '\r' && c != '\n') {
        chunkedDelimiterLine.push_back(c);
    }
    return AbstractHttpRequestParserBase::ParserAction::PA_CONTINUE;
}


} }
