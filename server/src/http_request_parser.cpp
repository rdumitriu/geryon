/*
 * HttpRequestParser.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */
#include <iostream>
#include <sstream>

#include "http_request_parser.hpp"

#include "string_utils.hpp"
#include "log.hpp"

namespace geryon { namespace server {

#define MAX_METHOD_LENGTH 16
#define MAX_URI_LENGTH 4096
#define MAX_HTTPVERSION_LENGTH 8
#define MAX_HTTPHEADERNAME_LENGTH 4096
#define MAX_HTTPHEADERVALUE_LENGTH 8192

HttpRequestParser::HttpRequestParser(std::size_t maximalContentLenght) :
                                        consumedChars(0),
                                        maxContentLength(maximalContentLenght),
                                        state(START),
                                        uriState(URI_START),
                                        pRequest(0),
                                        postParamsCharCount(0),
                                        multipartSeparatorIndex(0),
                                        startMultipartIndex(0),
                                        stopMultipartIndex(0) {
    LOG(geryon::util::Log::DEBUG) << "Parser created";
}

HttpRequestParser::~HttpRequestParser() {
}

std::size_t HttpRequestParser::maximalContentLenght() const {
    return maxContentLength;
}


void HttpRequestParser::init(geryon::server::HttpServerRequest *_pRequest) {
    pRequest = _pRequest;
    state = START;
    uriState = URI_START;
    consumedChars = 0;
    postParamsCharCount = 0;
    multipartSeparatorIndex = 0;
    headerName.clear();
    headerValue.clear();
    paramName.clear();
    paramValue.clear();
    multipartSeparator.clear();
    multipartHeaderLine.clear();
    multipartFileName.clear();
    multipartCountentType.clear();
    multipartCountentEncoding.clear();
    multipartContentQueue.clear();
    startMultipartIndex = 0;
    stopMultipartIndex = 0;
}

geryon::HttpResponse::HttpStatusCode HttpRequestParser::validate() {
    if(!validateMethod()) {
        return geryon::HttpResponse::SC_METHOD_NOT_ALLOWED;
    }
    //1: decode the URL
    std::string decodedURL;
    if(!geryon::util::decodeURL(pRequest->uri, decodedURL)) {
        return geryon::HttpResponse::SC_BAD_REQUEST;
    }
    pRequest->uri = decodedURL;
    //2: decode the URI
    decodedURL.clear();
    geryon::util::decodeURL(pRequest->queryString, decodedURL);
    pRequest->queryString = decodedURL;
    //3: decode the Path
    decodedURL.clear();
    geryon::util::decodeURL(pRequest->uriPath, decodedURL);
    pRequest->uriPath = decodedURL;
    //4: The content length or the Transfer-Encoding
    geryon::HttpResponse::HttpStatusCode ret = geryon::HttpResponse::SC_OK;
    bool hasTransferEncondingHeader = pRequest->hasHeader("Transfer-Encoding") || pRequest->hasHeader("Expect");
    bool mustCalculateMissingLength = false;

    if(hasTransferEncondingHeader) {
        //content length will be dynamically determined
        std::string tehstr = pRequest->getHeaderValue("Transfer-Encoding");
        if(tehstr != "" && tehstr != "chunked") {
            LOG(geryon::util::Log::ERROR) << "Not implemented, transfer encoding requested with:" << tehstr;
            return geryon::HttpResponse::SC_NOT_IMPLEMENTED;
        } else if(tehstr == "") {
            //TE is empty, we will have an 100-continue then we'll proceed as usual
            //so we must calculate Content-Length header
            mustCalculateMissingLength = true;
        }
        std::string expecthdr = pRequest->getHeaderValue("Expect");
        if(expecthdr != "100-continue") {
            LOG(geryon::util::Log::ERROR) << "Transfer encoding requested with invalid expect header:" << expecthdr;
            return geryon::HttpResponse::SC_BAD_REQUEST;
        }
        //signal that we have to send the 100 header, set the cnt-len to max
        LOG(geryon::util::Log::DEBUG) << "Should initiate chunked transfer encoding";
        pRequest->contentLength = maxContentLength;
        ret = geryon::HttpResponse::SC_CONTINUE;
    }
    //even if TE header is present, browsers should sent this also.
    //this is to be fully compliant with HTTP 1.1; however, we cannot rely on it
    bool hasContentLengthHeader = false;
    if(!hasTransferEncondingHeader || mustCalculateMissingLength) {
        hasContentLengthHeader = pRequest->hasHeader("Content-Length");
        if(hasContentLengthHeader) {
            std::string ctstr = pRequest->getHeaderValue("Content-Length");
            try {
                pRequest->contentLength = geryon::util::convertTo(ctstr, 0L);
                if(pRequest->contentLength > maxContentLength) {
                    LOG(geryon::util::Log::ERROR) << "Request too large :" << ctstr;
                    return geryon::HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
                }
            } catch (...) {
                LOG(geryon::util::Log::ERROR) << "Invalid content length on the request :" << ctstr;
                return geryon::HttpResponse::SC_BAD_REQUEST;
            }
        }
    }
    if((hasContentLengthHeader || hasTransferEncondingHeader) && pRequest->getMethodCode() == HttpRequest::TRACE) {
        LOG(geryon::util::Log::ERROR) << "Malformed request: TRACE request with a body.";
        return geryon::HttpResponse::SC_BAD_REQUEST;
    }
    //5: The content type
    if(pRequest->hasHeader("Content-Type")) {
        pRequest->contentType = pRequest->getHeaderValue("Content-Type");
    }
    //::TODO:: other checks
    return ret;
}

bool HttpRequestParser::consumeStart(char input, geryon::HttpResponse::HttpStatusCode & error) {
    if(!geryon::util::isHTTPChar(input) || geryon::util::isHTTPCtl(input) || geryon::util::isHTTPSpecial(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    state = METHOD;
    pRequest->method.push_back(input);
    return false;
}

bool HttpRequestParser::consumeMethod(char input, geryon::HttpResponse::HttpStatusCode & error) {
    if (input == ' ') {
      state = URI;
      return false;
    }
    else if(!geryon::util::isHTTPChar(input) || geryon::util::isHTTPCtl(input) || geryon::util::isHTTPSpecial(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    //GET,HEAD,POST,PUT,DELETE,TRACE,OPTIONS
    pRequest->method.push_back(input);
    if(pRequest->method.size() > MAX_METHOD_LENGTH) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    return false;
}

bool HttpRequestParser::consumeURI(char input, geryon::HttpResponse::HttpStatusCode & error) {
    consumeURISubstate(input);
    if(input == ' ') {
        state = HTTP_VERSION;
        return false;
    } else if(input == '\r') {
        state = NEWLINE_1;
        return false;
    } else if (geryon::util::isHTTPCtl(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    pRequest->uri.push_back(input);
    if(pRequest->uri.size() > MAX_URI_LENGTH) {
        error = geryon::HttpResponse::SC_REQUEST_URI_TOO_LONG;
        return true;
    }
    return false;
}

bool HttpRequestParser::consumeHttpVersion(char input, geryon::HttpResponse::HttpStatusCode & error) {
    if (input == '\r') {
        state = NEWLINE_1;
        return false;
    }
    else if (input == 'H' || input == 'T' || input == 'P' ||
             input == '/' || input == '.' || geryon::util::isHTTPDigit(input)) {
        pRequest->httpVersion.push_back(input);
        //HTTP/1.1 = 8 bytes
        if(pRequest->httpVersion.size() > MAX_HTTPVERSION_LENGTH) {
            error = geryon::HttpResponse::SC_BAD_REQUEST;
            return true;
        }
        return false;
    }
    error = geryon::HttpResponse::SC_BAD_REQUEST;
    return true;
}

bool HttpRequestParser::consumeChar(char expected, char input,geryon::HttpResponse::HttpStatusCode & error,
                                    HttpRequestParser::State nextState) {
    if (input == expected) {
        state = nextState;
        return false;
    }
    error = geryon::HttpResponse::SC_BAD_REQUEST;
    return true;
}

bool HttpRequestParser::consumeNewline(char input,geryon::HttpResponse::HttpStatusCode & error,
                                       HttpRequestParser::State nextState) {
    return consumeChar('\n', input, error, nextState);
}

bool HttpRequestParser::consumeHeaderName(char input,geryon::HttpResponse::HttpStatusCode & error) {
    if (input == '\r') {
        state = NEWLINE_2;
        return false;
    } else if(input == ':') {
        state = HEADER_VALUE_START;
        return false;
    } else if (input == ' ' || input == '\t') {
        state = HEADER_LWS;
        return false;
    } else if (!geryon::util::isHTTPChar(input) || geryon::util::isHTTPCtl(input) || geryon::util::isHTTPSpecial(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    headerName.push_back(input);
    if(headerName.size() > MAX_HTTPHEADERNAME_LENGTH) {
        error = geryon::HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
        return true;
    }
    return false;
}

bool HttpRequestParser::consumeHeaderLWS(char input,geryon::HttpResponse::HttpStatusCode & error) {
    if (input == '\r') {
        //named header only
        pRequest->addHeader(headerName, "");
        headerName.clear();
        headerValue.clear();
        state = NEWLINE_1;
        return false;
    } else if (input == ' ' || input == '\t' || geryon::util::isHTTPCtl(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    state = HEADER_VALUE;
    headerValue.push_back(input);
    if(headerValue.size() > MAX_HTTPHEADERVALUE_LENGTH) {
        error = geryon::HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
        return true;
    }
    return false;
}

bool HttpRequestParser::consumeHeaderValue(char input,geryon::HttpResponse::HttpStatusCode & error) {
    if (input == '\r') {
        //header is placed here in the message
        pRequest->addHeader(headerName, headerValue);
        headerName.clear();
        headerValue.clear();
        state = NEWLINE_1;
        return false;
    } else if (geryon::util::isHTTPCtl(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    headerValue.push_back(input);
    if(headerValue.size() > MAX_HTTPHEADERVALUE_LENGTH) {
        error = geryon::HttpResponse::SC_REQUEST_ENTITY_TOO_LARGE;
        return true;
    }
    return false;
}

bool HttpRequestParser::consumePostParameters(char input,geryon::HttpResponse::HttpStatusCode & error) {
    consumeURISubstate(input);
    countPostBytes();
    if(state == END && paramName.length()) {
        //last param, let's not forget about it
        pushRequestParameter();
    }
    return (state == END);
}

bool HttpRequestParser::consumePostMultipartBoundary(char input, geryon::HttpResponse::HttpStatusCode & error) {
    countPostBytes();
    if (input == '\r') {
        if(multipartSeparatorIndex == multipartSeparator.length()) {
            state = POST_MULTIPART_EATNEWLINE_1;
        }
        multipartSeparatorIndex = 0;
        return (state == END);
    } else if (geryon::util::isHTTPCtl(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    } else {
        std::size_t maxNdx = multipartSeparator.length() - 1;
        if(multipartSeparatorIndex > maxNdx ||
           multipartSeparator[multipartSeparatorIndex] != input) {
            error = geryon::HttpResponse::SC_BAD_REQUEST;
            return true;
        }
        multipartSeparatorIndex++;
        return (state == END);
    }
}

bool HttpRequestParser::consumePostMultipartNewline(char input, geryon::HttpResponse::HttpStatusCode & error, State nextState) {
    countPostBytes();
    return consumeNewline(input, error, state == END ? END : nextState);
}

bool HttpRequestParser::consumePostMultipartHeaderOrContent(char input, geryon::HttpResponse::HttpStatusCode & error) {
    countPostBytes();
    if(input == '\r') {
        //prepare for content, after the next newline
        state = (state == END) ? END : POST_MULTIPART_EATNEWLINE_3;
        return (state == END);
    } else if (geryon::util::isHTTPCtl(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    } else {
        multipartHeaderLine.push_back(input);
        state = state == END ? END : POST_MULTIPART_HEADER;
        return (state == END);
    }
}

bool HttpRequestParser::consumePostMultipartHeaderLine(char input, geryon::HttpResponse::HttpStatusCode & error) {
    countPostBytes();
    if (input == '\r') {
        if(!processMultipartHeader()) {
            error = geryon::HttpResponse::SC_BAD_REQUEST;
            return true;
        }
        LOG(geryon::util::Log::DEBUG) << "Multipart header line is [[" << multipartHeaderLine
                                      << "]] Param:" << paramName
                                      << " File:" << multipartFileName
                                      << " CT:" << multipartCountentType;
        multipartHeaderLine.clear();
        state = state == END ? END : POST_MULTIPART_EATNEWLINE_2;
    } else if (geryon::util::isHTTPCtl(input)) {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    } else {
        multipartHeaderLine.push_back(input);
    }
    return (state == END);
}

bool HttpRequestParser::consumePostMultipartContent(char input, geryon::HttpResponse::HttpStatusCode & error) {
    countPostBytes();
    multipartContentQueue.push_back(input);

    if(multipartContentQueue.size() < multipartSeparator.length() + 3) {
        //do nothing, we're just accumullating chars
        return (state == END);
    }
    char front = multipartContentQueue.front();
    multipartContentQueue.pop_front();
    if(multipartFileName.length()) {
        //first char pushed into it is the part start
        if(startMultipartIndex == 0) {
            startMultipartIndex = consumedChars - 1;
            stopMultipartIndex = startMultipartIndex;
        } else {
            ++stopMultipartIndex;
        }
    } else {
        //normal value
        paramValue.push_back(front);
    }
    if(multipartContentMatchesBoundary()) {
        multipartContentQueue.clear();
        if(multipartFileName.length()) {
            //add part
            std::shared_ptr<geryon::HttpRequestPart> ptr = std::make_shared<HttpServerRequestPart>(paramName,
                                                                                                   multipartFileName,
                                                                                                   multipartCountentType,
                                                                                                   pRequest,
                                                                                                   startMultipartIndex,
                                                                                                   stopMultipartIndex);
            pRequest->addPart(ptr);
        } else {
            //add normal param
            pRequest->addParameter(paramName, paramValue);
        }
        paramName.clear();
        paramValue.clear();
        multipartFileName.clear();
        multipartCountentType.clear();
        multipartCountentEncoding.clear();
        startMultipartIndex = 0;
        stopMultipartIndex = 0;
        state = state == END ? END : POST_MULTIPART_NEXT_PART_OR_END;
    }
    return (state == END);
}

bool HttpRequestParser::consumePostMultipartNextPartOrEnd(char input, geryon::HttpResponse::HttpStatusCode & error) {
    countPostBytes();
    if(input == '\r') {
        state = state == END ? END : POST_MULTIPART_EATNEWLINE_1;
        return (state == END);
    } else if(input == '-') {
        state = state == END ? END : POST_MULTIPART_END;
        return (state == END);
    } else {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
}

bool HttpRequestParser::consumePostMultipartEnd(char input, geryon::HttpResponse::HttpStatusCode & error) {
    countPostBytes();
    if(input != '-') {
        error = geryon::HttpResponse::SC_BAD_REQUEST;
        return true;
    }
    state = END; /* we can ignore everything from now on */
    return true;
}

const char * const WWWFORMURLENCODED = "application/x-www-form-urlencoded";
const char * const MULTIPARTFORMDATA = "multipart/form-data";

bool HttpRequestParser::consume(char c,geryon::HttpResponse::HttpStatusCode & error) {
    ++consumedChars;
    //LOG(Log::DEBUG) << "Consume char [" << c << "] State:" << state;
    switch(state) {
        case START:
            return consumeStart(c, error);
        case METHOD:
            return consumeMethod(c, error);
        case URI:
            return consumeURI(c, error);
        case HTTP_VERSION:
            return consumeHttpVersion(c, error);
        case NEWLINE_1:
            return consumeNewline(c, error, HEADER_NAME);
        case HEADER_NAME:
            return consumeHeaderName(c, error);
        case HEADER_LWS:
            return consumeHeaderLWS(c, error);
        case HEADER_VALUE_START:
            return consumeChar(' ', c, error, HEADER_VALUE);
        case HEADER_VALUE:
            return consumeHeaderValue(c, error);
        case NEWLINE_2:
            consumeNewline(c, error, END);
            error = validate();
            pRequest->setStreamStartIndex(consumedChars); //here is where the raw stream begins
            if((error == geryon::HttpResponse::SC_OK || error == geryon::HttpResponse::SC_CONTINUE) &&
               pRequest->getMethodCode() == geryon::HttpRequest::POST) {
                paramName.clear();
                paramValue.clear();

                if(geryon::util::startsWith(pRequest->contentType, WWWFORMURLENCODED)) {
                    //we have to read params from the body, so:
                    pRequest->queryString.push_back('&');
                    state = POST_PARAMS;
                    uriState = URI_PARAM_NAME;
                    return false;
                } else if(geryon::util::startsWith(pRequest->contentType, MULTIPARTFORMDATA)) {
                    //this is a multipart/form-data
                    //first extract the multipart separator from the header
                    if(!extractMultipartSeparator()) {
                        error = geryon::HttpResponse::SC_BAD_REQUEST;
                        return true;
                    }
                    //next, advance to the multipart boundary:
                    state = POST_MULTIPART_BOUNDARY;
                    return false;
                } else {
                    error = geryon::HttpResponse::SC_BAD_REQUEST;
                    return true;
                }
            }
            return true;
        case POST_PARAMS:
            return consumePostParameters(c, error);
        case POST_MULTIPART_BOUNDARY:
            return consumePostMultipartBoundary(c, error);
        case POST_MULTIPART_EATNEWLINE_1:
            return consumePostMultipartNewline(c, error, POST_MULTIPART_HEADER);
        case POST_MULTIPART_HEADER:
            return consumePostMultipartHeaderLine(c, error);
        case POST_MULTIPART_EATNEWLINE_2:
            return consumePostMultipartNewline(c, error, POST_MULTIPART_HEADER_OR_CONTENT);
        case POST_MULTIPART_HEADER_OR_CONTENT:
            return consumePostMultipartHeaderOrContent(c, error);
        case POST_MULTIPART_EATNEWLINE_3:
            return consumePostMultipartNewline(c, error, POST_MULTIPART_CONTENT);
        case POST_MULTIPART_CONTENT:
            return consumePostMultipartContent(c, error);
        case POST_MULTIPART_NEXT_PART_OR_END:
            return consumePostMultipartNextPartOrEnd(c, error);
        case POST_MULTIPART_END:
            return consumePostMultipartEnd(c, error);
        case END:
            return true;
        default:
            break;
    }
    error = geryon::HttpResponse::SC_BAD_REQUEST;
    return true;
}

bool HttpRequestParser::validateMethod() {
    geryon::util::toUpper(pRequest->method);
    //Supported: GET*,HEAD*,POST*,PUT*,DELETE*,TRACE*,OPTIONS*
    if(pRequest->method == "GET") {
        pRequest->methodCode = HttpRequest::GET;
    } else if(pRequest->method == "POST") {
        pRequest->methodCode = HttpRequest::POST;
    } else if(pRequest->method == "HEAD") {
        pRequest->methodCode = HttpRequest::HEAD;
    } else if(pRequest->method == "DELETE") {
        pRequest->methodCode = HttpRequest::DELETE;
    } else if(pRequest->method == "PUT") {
        pRequest->methodCode = HttpRequest::PUT;
    } else if(pRequest->method == "TRACE") {
        pRequest->methodCode = HttpRequest::TRACE;
    } else if(pRequest->method == "OPTIONS") {
        pRequest->methodCode = HttpRequest::OPTIONS;
    } else {
        return false;
    }
    return true;
}

bool HttpRequestParser::extractMultipartSeparator() {
    multipartSeparator = "--";
    //multipart/form-data; boundary=----893724sdaewq48
    if(pRequest->contentType.length() < 20) { //"multipart/form-data".lenght == 19
        return false;
    }
    std::size_t pos = pRequest->contentType.find('=', 19);
    if(pos == std::string::npos || pos == pRequest->contentType.length() - 1) {
        return false;
    }
    multipartSeparator.append(pRequest->contentType.c_str() + pos + 1);
    LOG(geryon::util::Log::DEBUG) << "Multipart separator is [[" << multipartSeparator << "]]";
    return true;
}

bool HttpRequestParser::multipartContentMatchesBoundary() {
    std::deque<char>::iterator i = multipartContentQueue.begin();
    if(*i != '\r') {
        return false;
    }
    ++i;
    if(*i != '\n') {
        return false;
    }
    ++i;
    std::size_t j = 0;
    for(; i != multipartContentQueue.end() && j < multipartSeparator.length(); ++i, ++j) {
        if(*i != multipartSeparator[j]) {
            return false;
        }
    }
    if(j != multipartSeparator.length()) {
        return false;
    }
    return true;
}

void HttpRequestParser::countPostBytes() {
    postParamsCharCount++;
    if(postParamsCharCount == pRequest->contentLength) {
        state = END;
    }
}

//Content-Disposition: form-data; name="paramname"
//Content-Disposition: form-data; name="thefileparamname"; filename="thefilename"
//Content-Type: application/octet-stream
//Content-Transfer-Encoding: 7bit
bool HttpRequestParser::processMultipartHeader() {
    if(geryon::util::startsWith(multipartHeaderLine, "Content-Disposition:")) {
        parseContentDispositionHeader();
        return true;
    } else if(geryon::util::startsWith(multipartHeaderLine, "Content-Type:")) {
        if(multipartHeaderLine.length() > 13) {
            multipartCountentType = geryon::util::trimAndCopy(multipartHeaderLine.substr(13));
            return true;
        } else {
            return false;
        }
    } else if(geryon::util::startsWith(multipartHeaderLine, "Content-Transfer-Encoding:")) {
        if( multipartHeaderLine.length() > 26) {
            multipartCountentEncoding = geryon::util::trimAndCopy(multipartHeaderLine.substr(26));
            return true;
        } else {
            return false;
        }
    } /* else : unrecognized header, ignored */
    return false;
}

void HttpRequestParser::parseContentDispositionHeader() {
    int status = 0; /* match token */
    std::string p;
    std::string v;
    for(std::size_t i = 20; i < multipartHeaderLine.length(); ++i) {
        char c = multipartHeaderLine[i];
        switch(status) {
            case 0: /* form-data;, skipped */
                if(c == ';') {
                    status = 1;
                } else {
                    continue;
                }
                break;
            case 1: /* match token */
                if(c == '=') {
                    status = 2;
                } else if( c == ' ' || c == '\t' || c == ';' || c == ':') {
                    continue;
                } else {
                    p.push_back(c);
                }
                break;
            case 2: /* expecting attr value */
                if( c == ' ' || c == '\t' ) {
                    continue;
                } else if(c == '"') {
                    status = 3;
                } else {
                    v.push_back(c);
                    status = 3;
                }
                break;
            case 3: /* value */
                if(c == '"') {
                    //interpret them
                    if(p == "name") {
                        paramName = v;
                    } else if (p == "filename") {
                        multipartFileName = v;
                    }
                    p.clear();
                    v.clear();
                    status = 1;
                } else {
                    v.push_back(c);
                }
                break;
        }
    }
}

// we have URLs like:
//=> /some/path[?param1=value1&param2=value+2]
//=> www.foo.com[:port]/some/path[?param1=value1&param2=value+2] (not standard)
//=> http://www.foo.com[:port]/some/path[?param1=value1&param2=value+2] (not standard)
bool HttpRequestParser::consumeURISubstate(char input) {
    switch( uriState) {
        case URI_START:
            {
                if(input == '/') {
                    uriState = URI_PATH; /* normal */
                    pRequest-> uriPath.push_back(input);
                } else {
                    uriState = URI_PROTO;
                    pRequest-> uriProtocol.push_back(input);
                }
            }
            break;
        case URI_PROTO: /*should not appear normally */
            {
                if(input == ':') {
                    uriState = URI_CONSUME_SLASH;
                } else {
                    pRequest-> uriProtocol.push_back(input);
                }
            }
            break;
        case URI_CONSUME_SLASH: /* should not appear normally */
            {
                if(input != '/') {
                    uriState = URI_HOST;
                    pRequest-> uriHost.push_back(input);
                }
            }
            break;
        case URI_HOST: /* should not appear normally */
            {
                if(input == ':') {
                    uriState = URI_PORT;
                } else if(input == '/') {
                    uriState = URI_PATH;
                    pRequest-> uriPath.push_back(input);
                } else {
                    pRequest-> uriHost.push_back(input);
                }
            }
            break;
        case URI_PORT: /* should not appear normally */
            {
                if(input == '/') {
                    uriState = URI_PATH;
                    pRequest-> uriPath.push_back(input);
                } else {
                    pRequest-> uriPort.push_back(input);
                }
            }
            break;
        case URI_PATH: /* this is what we actually expect all the time */
            {
                if(input == ' ' || input == '\r') {
                    uriState = URI_END;
                } else if(input == '?') {
                    uriState = URI_PARAM_NAME;
                } else {
                    pRequest-> uriPath.push_back(input);
                }
            }
            break;
        case URI_PARAM_NAME:
            {
                pRequest-> queryString.push_back(input);
                if(input == '&') {
                    //does nothing, some browsers wrongly encodes '&' at the
                    //beginning of the parameter name
                } else if(input == '=') {
                    uriState = URI_PARAM_VALUE;
                } else {
                    paramName.push_back(input);
                }
            }
            break;
        case URI_PARAM_VALUE:
            {
                if(input == '&') {
                    uriState = URI_PARAM_NAME;
                    pRequest-> queryString.push_back(input);
                    pushRequestParameter();
                    //next param (maybe)
                } else if(input == ' ' || input == '\r') {
                    pushRequestParameter();
                    uriState = URI_END;
                    //end it
                } else {
                    pRequest-> queryString.push_back(input);
                    paramValue.push_back(input);
                }
            }
            break;
        case URI_END:
            break;
    }
    return true;
}

void HttpRequestParser::pushRequestParameter() {
    std::string n;
    std::string v;
    if( paramName.length()) {
        geryon::util::decodeURL( paramName, n);
        geryon::util::decodeURL( paramValue, v);
        pRequest->addParameter(n, v);
    }
    paramName.clear();
    paramValue.clear();
}

} } /*namespace */
