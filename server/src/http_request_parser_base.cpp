///
/// \file http_request_parser_base.cpp
///
///  Created on: Aug 25, 2014
///      Author: rdumitriu at gmail.com
///

#include "http_request_parser_base.hpp"

namespace geryon { namespace server {

AbstractHttpRequestParserBase::AbstractHttpRequestParserBase(std::size_t _maximalContentLenght,
                                                             std::size_t & _absoluteIndex)
                : maxContentLength(_maximalContentLenght), absoluteIndex(_absoluteIndex) {
}

AbstractHttpRequestParserBase::~AbstractHttpRequestParserBase() {}

void AbstractHttpRequestParserBase::init(geryon::server::HttpServerRequest *_pRequest) {
    pRequest = _pRequest;
    //if(pRequest) {
        //pRequest->clear(); //implement this
    //}
}

AbstractHttpRequestParserBase::ParserAction
AbstractHttpRequestParserBase::consume(char c, geryon::HttpResponse::HttpStatusCode & error) {
    if(absoluteIndex > maxContentLength) {
        error = HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
        return ParserAction::PA_DONE;
    }
    return ParserAction::PA_CONTINUE;
}


} }
