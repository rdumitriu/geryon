///
/// \file http_request_parser.hpp
///
///  Created on: Aug 25, 2011
///     Author: rdumitriu at gmail.com
///
 
#ifndef GERYON_HTTPREQUEST_PARSER_HPP_
#define GERYON_HTTPREQUEST_PARSER_HPP_

#include <cstdio>
#include <string>
#include <deque>

#include "http_server_types.hpp"
#include "http_request_parser_base.hpp"
 
namespace geryon { namespace server {


class HttpRequestParser : public AbstractHttpRequestParserBase {
public:
    /// Constructor
    HttpRequestParser(std::size_t maximalContentLenght, std::size_t & absoluteIndex);
    
    ///Destructor
    virtual ~HttpRequestParser();

    /// Reset to initial parser state.
    virtual void init(geryon::server::HttpServerRequest *pRequest);
    
    ///consume a char from input
    virtual ParserAction consume(char c, geryon::HttpResponse::HttpStatusCode & error);

private:

    /// The current state of the parser.
    enum State {
        START,
        METHOD,
        URI,
        HTTP_VERSION,
        NEWLINE_1,
        HEADER_NAME,
        HEADER_LWS,
        HEADER_VALUE_START,
        HEADER_VALUE,
        NEWLINE_2,
        POST_PARAMS,
        POST_MULTIPART_START,
        POST_MULTIPART_BOUNDARY,
        POST_MULTIPART_EATNEWLINE_1,
        POST_MULTIPART_HEADER,
        POST_MULTIPART_EATNEWLINE_2,
        POST_MULTIPART_HEADER_OR_CONTENT,
        POST_MULTIPART_EATNEWLINE_3,
        POST_MULTIPART_CONTENT,
        POST_MULTIPART_NEXT_PART_OR_END,
        POST_MULTIPART_END,
        REMAINDER,
        END
    };
    
    /// The current state of the URI subparser
    enum URIState {
        URI_START,
        URI_PROTO,
        URI_CONSUME_SLASH,
        URI_HOST,
        URI_PORT,
        URI_PATH,
        URI_PARAM_NAME,
        URI_PARAM_VALUE,
        URI_END
    };

    State state;
    URIState uriState;
    std::size_t postParamsCharCount;
    
    std::string headerName;
    std::string headerValue;

    ///Call this after you parsed the message
    geryon::HttpResponse::HttpStatusCode validate();
    
    //Parser
    ParserAction consumeChar(char expected, char input, geryon::HttpResponse::HttpStatusCode & error, State nextState);
    ParserAction consumeStart(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumeMethod(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumeURI(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumeHttpVersion(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumeNewline(char input, geryon::HttpResponse::HttpStatusCode & error, State nextState);
    ParserAction consumeHeaderName(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumeHeaderLWS(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumeHeaderValue(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumePostParameters(char input, geryon::HttpResponse::HttpStatusCode & error);
    
    ParserAction consumePostMultipartBoundary(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumePostMultipartNewline(char input, geryon::HttpResponse::HttpStatusCode & error, State nextState);
    ParserAction consumePostMultipartHeaderOrContent(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumePostMultipartHeaderLine(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumePostMultipartContent(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumePostMultipartNextPartOrEnd(char input, geryon::HttpResponse::HttpStatusCode & error);
    ParserAction consumePostMultipartEnd(char input, geryon::HttpResponse::HttpStatusCode & error);

    ParserAction consumeUninterpretedRemainder(char input);
    
    //URI_break_down
    std::string paramName;
    std::string paramValue;
    bool consumeURISubstate(char c);
    void pushRequestParameter();
    bool validateMethod();
    
    //Multipart:
    std::string multipartSeparator;
    std::size_t multipartSeparatorIndex;
    std::string multipartHeaderLine;
    std::string multipartFileName;
    std::string multipartCountentType;
    std::string multipartCountentEncoding;
    std::size_t startMultipartIndex;
    std::size_t stopMultipartIndex;
    
    std::deque<char> multipartContentQueue;

    bool extractMultipartSeparator();
    bool multipartContentMatchesBoundary();
    void countPostBytes();
    bool processMultipartHeader();
    void parseContentDispositionHeader();
};

} } /* namespace */

#endif
