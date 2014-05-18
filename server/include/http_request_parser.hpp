/*
 * HTTPRequestParser.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */
 
#ifndef HTTPREQUEST_PARSER_HPP_
#define HTTPREQUEST_PARSER_HPP_

#include <cstdio>
#include <string>
#include <deque>

#include "http_server_types.hpp"
 
namespace geryon { namespace server {

class HttpRequestParser {
public:
    /// Constructor
    HttpRequestParser(std::size_t maximalContentLenght);
    
    ///Destructor
    ~HttpRequestParser();

    /// Reset to initial parser state.
    void init(geryon::server::HttpServerRequest *pRequest);
    
    ///consume a char from input
    bool consume(char c, geryon::HttpResponse::HttpStatusCode & error);
    
    ///Call this after you parsed the message
    geryon::HttpResponse::HttpStatusCode validate();

    ///Report back the maximal content length
    std::size_t maximalContentLenght() const;
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
    
    std::size_t consumedChars; //counts the number of chars consumed.
    std::size_t maxContentLength;

    State state;
    URIState uriState;
    geryon::server::HttpServerRequest *pRequest;
    std::size_t postParamsCharCount;
    
    std::string headerName;
    std::string headerValue;
    
    bool consumeChar(char expected, char input, geryon::HttpResponse::HttpStatusCode & error, State nextState);
    bool consumeStart(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumeMethod(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumeURI(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumeHttpVersion(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumeNewline(char input, geryon::HttpResponse::HttpStatusCode & error, State nextState);
    bool consumeHeaderName(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumeHeaderLWS(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumeHeaderValue(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumePostParameters(char input, geryon::HttpResponse::HttpStatusCode & error);
    
    bool consumePostMultipartBoundary(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumePostMultipartNewline(char input, geryon::HttpResponse::HttpStatusCode & error, State nextState);
    bool consumePostMultipartHeaderOrContent(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumePostMultipartHeaderLine(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumePostMultipartContent(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumePostMultipartNextPartOrEnd(char input, geryon::HttpResponse::HttpStatusCode & error);
    bool consumePostMultipartEnd(char input, geryon::HttpResponse::HttpStatusCode & error);
    
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
