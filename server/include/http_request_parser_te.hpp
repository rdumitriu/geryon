///
/// \file http_request_parser_te.hpp
///
///  Created on: Sep 29, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_HTTPREQUEST_PARSER_TE_HPP_
#define GERYON_HTTPREQUEST_PARSER_TE_HPP_

#include "http_server_types.hpp"
#include "http_request_parser_base.hpp"

namespace geryon { namespace server {

class HttpRequestParserChunkedTE : public AbstractHttpRequestParserBase {
public:
    HttpRequestParserChunkedTE(std::shared_ptr<AbstractHttpRequestParserBase> pRealParser,
                               std::size_t maximalContentLenght, std::size_t & absoluteIndex);
    ///Destructor
    virtual ~HttpRequestParserChunkedTE();

    ///consume a char from input
    virtual ParserAction consume(char c, geryon::HttpResponse::HttpStatusCode & error);
private:
    AbstractHttpRequestParserBase::ParserAction
    parseChunkedTESize(char c, geryon::HttpResponse::HttpStatusCode & error);

    std::shared_ptr<AbstractHttpRequestParserBase> pRealParser;
    std::string chunkedDelimiterLine;
    std::size_t chunkSize;
    std::size_t chunkTransferredSz;
    unsigned int chunkedState;
    std::size_t gapStart;

};

} }

#endif
