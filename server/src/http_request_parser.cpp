/*
 * HTTPRequestParser.hpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */
#include <iostream>
#include <sstream>

#include <HTTPRequestParser.hpp>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <Log.hpp>

namespace agrade { namespace yahsrv { namespace server {

#define MAX_METHOD_LENGTH 16
#define MAX_URI_LENGTH 4096
#define MAX_HTTPVERSION_LENGTH 8
#define MAX_HTTPHEADERNAME_LENGTH 4096
#define MAX_HTTPHEADERVALUE_LENGTH 8192

//::TODO:: yet another point where we should take care of WIN32 stuff (/tmp)
HTTPRequestParser::HTTPRequestParser(std::size_t maximalCT,
                                     std::size_t fileIOBufferSize,
                                     const std::string & tempDir) :
                                     m_maxContentLength(maximalCT), m_bufferSize(4096), m_tmpDir("/tmp"),
                                         m_state(START),
                                         m_uriState(URI_START),
                                         m_pRequest(NULL),
                                         m_postParamsCharCount(0),
                                         m_multipartSeparatorIndex(0),
                                         m_mFile(NULL),
                                         m_mFileWriteCount(0),
                                         m_mFileBuffer(NULL),
                                         m_mFileCounter(0) {
}

HTTPRequestParser::~HTTPRequestParser() {
    closePartFile();
    delete m_mFileBuffer;
}

void HTTPRequestParser::reset(agrade::yahsrv::HTTPRequest *pRequest) {
    m_pRequest = pRequest;
    m_state = START;
    m_uriState = URI_START;
    m_postParamsCharCount = 0;
    m_multipartSeparatorIndex = 0;
    m_headerName.clear();
    m_headerValue.clear();
    m_paramName.clear();
    m_paramValue.clear();
    m_multipartSeparator.clear();
    m_multipartHeaderLine.clear();
    m_multipartFileName.clear();
    m_multipartCountentType.clear();
    m_multipartCountentEncoding.clear();
    m_multipartContentQueue.clear();
    closePartFile();
}

agrade::yahsrv::HTTPReply::HTTPStatusCode HTTPRequestParser::validate() {
    if(!validateMethod()) {
        return agrade::yahsrv::HTTPReply::SC_METHOD_NOT_ALLOWED;
    }
    //1: decode the URL
    std::string decodedURL;
    if(!decodeURL(m_pRequest->m_uri, decodedURL)) {
        return agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
    }
    m_pRequest->m_uri = decodedURL;
    //2: decode the URI
    decodedURL.clear();
    decodeURL(m_pRequest->m_queryString, decodedURL);
    m_pRequest->m_queryString = decodedURL;
    //3: decode the Path
    decodedURL.clear();
    decodeURL(m_pRequest->m_uriPath, decodedURL);
    m_pRequest->m_uriPath = decodedURL;
    //4: The content length or the Transfer-Encoding
    agrade::yahsrv::HTTPReply::HTTPStatusCode ret = agrade::yahsrv::HTTPReply::SC_OK;
    bool hasTransferEncondingHeader = m_pRequest->hasHeader("Transfer-Encoding") || m_pRequest->hasHeader("Expect");
    bool mustCalculateMissingLength = false;

    if(hasTransferEncondingHeader) {
        //content length will be dynamically determined
        std::string tehstr = m_pRequest->getHeaderValue("Transfer-Encoding");
        if(tehstr != "" && tehstr != "chunked") {
            LOG(agrade::util::Log::ERROR) << "Not implemented, transfer encoding requested with:" << tehstr;
            return agrade::yahsrv::HTTPReply::SC_NOT_IMPLEMENTED;
        } else if(tehstr == "") {
            //TE is empty, we will have an 100-continue then we'll proceed as usual
            //so we must calculate Content-Length header
            mustCalculateMissingLength = true;
        }
        std::string expecthdr = m_pRequest->getHeaderValue("Expect");
        if(expecthdr != "100-continue") {
            LOG(agrade::util::Log::ERROR) << "Transfer encoding requested with invalid expect header:" << expecthdr;
            return agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        }
        //signal that we have to send the 100 header, set the cnt-len to max
        LOG(agrade::util::Log::DEBUG) << "Should initiate chunked transfer encoding";
        m_pRequest->m_contentLength = m_maxContentLength;
        ret = agrade::yahsrv::HTTPReply::SC_CONTINUE;
    }
    //even if TE header is present, browsers should sent this also.
    //this is to be fully compliant with HTTP 1.1; however, we cannot rely on it
    bool hasContentLengthHeader = false;
    if(!hasTransferEncondingHeader || mustCalculateMissingLength) {
        hasContentLengthHeader = m_pRequest->hasHeader("Content-Length");
        if(hasContentLengthHeader) {
            std::string ctstr = m_pRequest->getHeaderValue("Content-Length");
            try {
                m_pRequest->m_contentLength = boost::lexical_cast<std::size_t>(ctstr);
                if(m_pRequest->m_contentLength > m_maxContentLength) {
                    LOG(agrade::util::Log::ERROR) << "Request too large :" << ctstr;
                    return agrade::yahsrv::HTTPReply::SC_REQUEST_ENTITY_TOO_LARGE;
                }
            } catch (...) {
                LOG(agrade::util::Log::ERROR) << "Invalid content length on the request :" << ctstr;
                return agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
            }
        }
    }
    if((hasContentLengthHeader || hasTransferEncondingHeader) && m_pRequest->getMethodCode() == HTTPRequest::TRACE) {
        LOG(agrade::util::Log::ERROR) << "Malformed request: TRACE request with a body.";
        return agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
    }
    //5: The content type
    if(m_pRequest->hasHeader("Content-Type")) {
        m_pRequest->m_contentType = m_pRequest->getHeaderValue("Content-Type");
    }
    //::TODO:: other checks
    return ret;
}

bool HTTPRequestParser::consumeStart(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    if(!isHTTPChar(input) || isHTTPCtl(input) || isHTTPSpecial(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    m_state = METHOD;
    m_pRequest->m_method.push_back(input);
    return false;
}

bool HTTPRequestParser::consumeMethod(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    if (input == ' ') {
      m_state = URI;
      return false;
    }
    else if(!isHTTPChar(input) || isHTTPCtl(input) || isHTTPSpecial(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    //GET,HEAD,POST,PUT,DELETE,TRACE,OPTIONS
    m_pRequest->m_method.push_back(input);
    if(m_pRequest->m_method.size() > MAX_METHOD_LENGTH) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    return false;
}

bool HTTPRequestParser::consumeURI(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    consumeURISubstate(input);
    if(input == ' ') {
        m_state = HTTP_VERSION;
        return false;
    } else if(input == '\r') {
        m_state = NEWLINE_1;
        return false;
    } else if (isHTTPCtl(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    m_pRequest->m_uri.push_back(input);
    if(m_pRequest->m_uri.size() > MAX_URI_LENGTH) {
        error = agrade::yahsrv::HTTPReply::SC_REQUEST_URI_TOO_LONG;
        return true;
    }
    return false;
}

bool HTTPRequestParser::consumeHttpVersion(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    if (input == '\r') {
        m_state = NEWLINE_1;
        return false;
    }
    else if (input == 'H' || input == 'T' || input == 'P' ||
             input == '/' || input == '.' || isHTTPDigit(input)) {
        m_pRequest->m_httpVersion.push_back(input);
        //HTTP/1.1 = 8 bytes
        if(m_pRequest->m_httpVersion.size() > MAX_HTTPVERSION_LENGTH) {
            error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
            return true;
        }
        return false;
    }
    error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
    return true;
}

bool HTTPRequestParser::consumeChar(char expected, char input,agrade::yahsrv::HTTPReply::HTTPStatusCode & error,
                                    HTTPRequestParser::State nextState) {
    if (input == expected) {
        m_state = nextState;
        return false;
    }
    error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
    return true;
}

bool HTTPRequestParser::consumeNewline(char input,agrade::yahsrv::HTTPReply::HTTPStatusCode & error,
                                       HTTPRequestParser::State nextState) {
    return consumeChar('\n', input, error, nextState);
}

bool HTTPRequestParser::consumeHeaderName(char input,agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    if (input == '\r') {
        m_state = NEWLINE_2;
        return false;
    } else if(input == ':') {
        m_state = HEADER_VALUE_START;
        return false;
    } else if (input == ' ' || input == '\t') {
        m_state = HEADER_LWS;
        return false;
    } else if (!isHTTPChar(input) || isHTTPCtl(input) || isHTTPSpecial(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    m_headerName.push_back(input);
    if(m_headerName.size() > MAX_HTTPHEADERNAME_LENGTH) {
        error = agrade::yahsrv::HTTPReply::SC_REQUEST_ENTITY_TOO_LARGE;
        return true;
    }
    return false;
}

bool HTTPRequestParser::consumeHeaderLWS(char input,agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    if (input == '\r') {
        //named header only
        m_pRequest->addHeader(m_headerName, "");
        m_headerName.clear();
        m_headerValue.clear();
        m_state = NEWLINE_1;
        return false;
    } else if (input == ' ' || input == '\t' || isHTTPCtl(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    m_state = HEADER_VALUE;
    m_headerValue.push_back(input);
    if(m_headerValue.size() > MAX_HTTPHEADERVALUE_LENGTH) {
        error = agrade::yahsrv::HTTPReply::SC_REQUEST_ENTITY_TOO_LARGE;
        return true;
    }
    return false;
}

bool HTTPRequestParser::consumeHeaderValue(char input,agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    if (input == '\r') {
        //header is placed here in the message
        m_pRequest->addHeader(m_headerName, m_headerValue);
        m_headerName.clear();
        m_headerValue.clear();
        m_state = NEWLINE_1;
        return false;
    } else if (isHTTPCtl(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    m_headerValue.push_back(input);
    if(m_headerValue.size() > MAX_HTTPHEADERVALUE_LENGTH) {
        error = agrade::yahsrv::HTTPReply::SC_REQUEST_ENTITY_TOO_LARGE;
        return true;
    }
    return false;
}

bool HTTPRequestParser::consumePostParameters(char input,agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    consumeURISubstate(input);
    countPostBytes();
    if(m_state == END && m_paramName.length()) {
        //last param, let's not forget about it
        pushRequestParameter();
    }
    return (m_state == END);
}

bool HTTPRequestParser::consumePostMultipartBoundary(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    countPostBytes();
    if (input == '\r') {
        if(m_multipartSeparatorIndex == m_multipartSeparator.length()) {
            m_state = POST_MULTIPART_EATNEWLINE_1;
        }
        m_multipartSeparatorIndex = 0;
        return (m_state == END);
    } else if (isHTTPCtl(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    } else {
        std::size_t maxNdx = m_multipartSeparator.length() - 1;
        if(m_multipartSeparatorIndex > maxNdx ||
           m_multipartSeparator[m_multipartSeparatorIndex] != input) {
            error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
            return true;
        }
        m_multipartSeparatorIndex++;
        return (m_state == END);
    }
}

bool HTTPRequestParser::consumePostMultipartNewline(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error, State nextState) {
    countPostBytes();
    return consumeNewline(input, error, m_state == END ? END : nextState);
}

bool HTTPRequestParser::consumePostMultipartHeaderOrContent(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    countPostBytes();
    if(input == '\r') {
        //prepare for content, after the next newline
        m_state = (m_state == END) ? END : POST_MULTIPART_EATNEWLINE_3;
        return (m_state == END);
    } else if (isHTTPCtl(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    } else {
        m_multipartHeaderLine.push_back(input);
        m_state = m_state == END ? END : POST_MULTIPART_HEADER;
        return (m_state == END);
    }
}

bool HTTPRequestParser::consumePostMultipartHeaderLine(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    countPostBytes();
    if (input == '\r') {
        if(!processMultipartHeader()) {
            error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
            return true;
        }
        LOG(agrade::util::Log::DEBUG) << "Multipart header line is [[" << m_multipartHeaderLine
                                      << "]] Param:" << m_paramName
                                      << " File:" << m_multipartFileName
                                      << " CT:" << m_multipartCountentType;
        m_multipartHeaderLine.clear();
        m_state = m_state == END ? END : POST_MULTIPART_EATNEWLINE_2;
    } else if (isHTTPCtl(input)) {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    } else {
        m_multipartHeaderLine.push_back(input);
    }
    return (m_state == END);
}

bool HTTPRequestParser::consumePostMultipartContent(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    countPostBytes();
    m_multipartContentQueue.push_back(input);

    if(m_multipartContentQueue.size() < m_multipartSeparator.length() + 3) {
        //do nothing, we're just accumullating chars
        return (m_state == END);
    }
    char front = m_multipartContentQueue.front();
    m_multipartContentQueue.pop_front();
    if(m_multipartFileName.length()) {
        //push into a file
        if(!writeIntoPartFile(front)) {
            closePartFile();
            m_state = END;
            error = agrade::yahsrv::HTTPReply::SC_INTERNAL_SERVER_ERROR;
            return true;
        }
    } else {
        //normal value
        m_paramValue.push_back(front);
    }
    if(multipartContentMatchesBoundary()) {
        m_multipartContentQueue.clear();
        if(m_multipartFileName.length()) {
            //add part
            agrade::yahsrv::HTTPRequestPart part;
            part.m_contentType = m_multipartCountentType;
            part.m_name = m_paramName;
            part.m_fileName = m_multipartFileName;
            part.m_realFileName = m_mFileName;
            m_pRequest->addPart(part);
            closePartFile();
        } else {
            //add normal param
            m_pRequest->addParameter(m_paramName, m_paramValue);
        }
        m_paramName.clear();
        m_paramValue.clear();
        m_multipartFileName.clear();
        m_multipartCountentType.clear();
        m_multipartCountentEncoding.clear();
        m_state = m_state == END ? END : POST_MULTIPART_NEXT_PART_OR_END;
    }
    return (m_state == END);
}

bool HTTPRequestParser::consumePostMultipartNextPartOrEnd(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    countPostBytes();
    if(input == '\r') {
        m_state = m_state == END ? END : POST_MULTIPART_EATNEWLINE_1;
        return (m_state == END);
    } else if(input == '-') {
        m_state = m_state == END ? END : POST_MULTIPART_END;
        return (m_state == END);
    } else {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
}

bool HTTPRequestParser::consumePostMultipartEnd(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    countPostBytes();
    if(input != '-') {
        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
        return true;
    }
    m_state = END; /* we can ignore everything from now on */
    return true;
}

const char * const WWWFORMURLENCODED = "application/x-www-form-urlencoded";
const char * const MULTIPARTFORMDATA = "multipart/form-data";

bool HTTPRequestParser::consume(char c,agrade::yahsrv::HTTPReply::HTTPStatusCode & error) {
    //LOG(Log::DEBUG) << "Consume char [" << c << "] State:" << m_state;
    switch(m_state) {
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
            if((error == agrade::yahsrv::HTTPReply::SC_OK || error == agrade::yahsrv::HTTPReply::SC_CONTINUE) &&
               m_pRequest->getMethodCode() == agrade::yahsrv::HTTPRequest::POST) {
                m_paramName.clear();
                m_paramValue.clear();

                if(boost::istarts_with(m_pRequest->m_contentType, WWWFORMURLENCODED)) {
                    //we have to read params from the body, so:
                    m_pRequest->m_queryString.push_back('&');
                    m_state = POST_PARAMS;
                    m_uriState = URI_PARAM_NAME;
                    return false;
                } else if(boost::istarts_with(m_pRequest->m_contentType, MULTIPARTFORMDATA)) {
                    //this is a multipart/form-data
                    //first extract the multipart separator from the header
                    if(!extractMultipartSeparator()) {
                        error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
                        return true;
                    }
                    //next, advance to the multipart boundary:
                    m_state = POST_MULTIPART_BOUNDARY;
                    return false;
                } else {
                    error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
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
    error = agrade::yahsrv::HTTPReply::SC_BAD_REQUEST;
    return true;
}

bool HTTPRequestParser::validateMethod() {
    boost::to_upper(m_pRequest->m_method);
    //Supported: GET*,HEAD*,POST*,PUT*,DELETE*,TRACE*,OPTIONS*
    if(m_pRequest->m_method == "GET") {
        m_pRequest->m_methodCode = HTTPRequest::GET;
    } else if(m_pRequest->m_method == "POST") {
        m_pRequest->m_methodCode = HTTPRequest::POST;
    } else if(m_pRequest->m_method == "HEAD") {
        m_pRequest->m_methodCode = HTTPRequest::HEAD;
    } else if(m_pRequest->m_method == "DELETE") {
        m_pRequest->m_methodCode = HTTPRequest::DELETE;
    } else if(m_pRequest->m_method == "PUT") {
        m_pRequest->m_methodCode = HTTPRequest::PUT;
    } else if(m_pRequest->m_method == "TRACE") {
        m_pRequest->m_methodCode = HTTPRequest::TRACE;
    } else if(m_pRequest->m_method == "OPTIONS") {
        m_pRequest->m_methodCode = HTTPRequest::OPTIONS;
    } else {
        return false;
    }
    return true;
}

bool HTTPRequestParser::extractMultipartSeparator() {
    m_multipartSeparator = "--";
    //multipart/form-data; boundary=----893724sdaewq48
    if(m_pRequest->m_contentType.length() < 20) { //"multipart/form-data".lenght == 19
        return false;
    }
    std::size_t pos = m_pRequest->m_contentType.find('=', 19);
    if(pos == std::string::npos || pos == m_pRequest->m_contentType.length() - 1) {
        return false;
    }
    m_multipartSeparator.append(m_pRequest->m_contentType.c_str() + pos + 1);
    LOG(agrade::util::Log::DEBUG) << "Multipart separator is [[" << m_multipartSeparator << "]]";
    return true;
}

bool HTTPRequestParser::multipartContentMatchesBoundary() {
    std::deque<char>::iterator i = m_multipartContentQueue.begin();
    if(*i != '\r') {
        return false;
    }
    ++i;
    if(*i != '\n') {
        return false;
    }
    ++i;
    std::size_t j = 0;
    for(; i != m_multipartContentQueue.end() && j < m_multipartSeparator.length(); ++i, ++j) {
        if(*i != m_multipartSeparator[j]) {
            return false;
        }
    }
    if(j != m_multipartSeparator.length()) {
        return false;
    }
    return true;
}

void HTTPRequestParser::countPostBytes() {
    m_postParamsCharCount++;
    if(m_postParamsCharCount == m_pRequest->m_contentLength) {
        m_state = END;
    }
}

//Content-Disposition: form-data; name="paramname"
//Content-Disposition: form-data; name="thefileparamname"; filename="thefilename"
//Content-Type: application/octet-stream
//Content-Transfer-Encoding: 7bit
bool HTTPRequestParser::processMultipartHeader() {
    if(boost::starts_with(m_multipartHeaderLine, "Content-Disposition:")) {
        parseContentDispositionHeader();
        return true;
    } else if(boost::starts_with(m_multipartHeaderLine, "Content-Type:")) {
        if(m_multipartHeaderLine.length() > 13) {
            m_multipartCountentType = boost::trim_copy(m_multipartHeaderLine.substr(13));
            return true;
        } else {
            return false;
        }
    } else if(boost::starts_with(m_multipartHeaderLine, "Content-Transfer-Encoding:")) {
        if(m_multipartHeaderLine.length() > 26) {
            m_multipartCountentEncoding = boost::trim_copy(m_multipartHeaderLine.substr(26));
            return true;
        } else {
            return false;
        }
    } /* else : unrecognized header, ignored */
    return false;
}

void HTTPRequestParser::parseContentDispositionHeader() {
    int status = 0; /* match token */
    std::string p;
    std::string v;
    for(std::size_t i = 20; i < m_multipartHeaderLine.length(); ++i) {
        char c = m_multipartHeaderLine[i];
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
                        m_paramName = v;
                    } else if (p == "filename") {
                        m_multipartFileName = v;
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
bool HTTPRequestParser::consumeURISubstate(char input) {
    switch(m_uriState) {
        case URI_START:
            {
                if(input == '/') {
                    m_uriState = URI_PATH; /* normal */
                    m_pRequest->m_uriPath.push_back(input);
                } else {
                    m_uriState = URI_PROTO;
                    m_pRequest->m_uriProtocol.push_back(input);
                }
            }
            break;
        case URI_PROTO: /*should not appear normally */
            {
                if(input == ':') {
                    m_uriState = URI_CONSUME_SLASH;
                } else {
                    m_pRequest->m_uriProtocol.push_back(input);
                }
            }
            break;
        case URI_CONSUME_SLASH: /* should not appear normally */
            {
                if(input != '/') {
                    m_uriState = URI_HOST;
                    m_pRequest->m_uriHost.push_back(input);
                }
            }
            break;
        case URI_HOST: /* should not appear normally */
            {
                if(input == ':') {
                    m_uriState = URI_PORT;
                } else if(input == '/') {
                    m_uriState = URI_PATH;
                    m_pRequest->m_uriPath.push_back(input);
                } else {
                    m_pRequest->m_uriHost.push_back(input);
                }
            }
            break;
        case URI_PORT: /* should not appear normally */
            {
                if(input == '/') {
                    m_uriState = URI_PATH;
                    m_pRequest->m_uriPath.push_back(input);
                } else {
                    m_pRequest->m_uriPort.push_back(input);
                }
            }
            break;
        case URI_PATH: /* this is what we actually expect all the time */
            {
                if(input == ' ' || input == '\r') {
                    m_uriState = URI_END;
                } else if(input == '?') {
                    m_uriState = URI_PARAM_NAME;
                } else {
                    m_pRequest->m_uriPath.push_back(input);
                }
            }
            break;
        case URI_PARAM_NAME:
            {
                m_pRequest->m_queryString.push_back(input);
                if(input == '&') {
                    //does nothing, some browsers wrongly encodes '&' at the
                    //beginning of the parameter name
                } else if(input == '=') {
                    m_uriState = URI_PARAM_VALUE;
                } else {
                    m_paramName.push_back(input);
                }
            }
            break;
        case URI_PARAM_VALUE:
            {
                if(input == '&') {
                    m_uriState = URI_PARAM_NAME;
                    m_pRequest->m_queryString.push_back(input);
                    pushRequestParameter();
                    //next param (maybe)
                } else if(input == ' ' || input == '\r') {
                    pushRequestParameter();
                    m_uriState = URI_END;
                    //end it
                } else {
                    m_pRequest->m_queryString.push_back(input);
                    m_paramValue.push_back(input);
                }
            }
            break;
        case URI_END:
            break;
    }
    return true;
}

void HTTPRequestParser::pushRequestParameter() {
    std::string n;
    std::string v;
    if(m_paramName.length()) {
        decodeURL(m_paramName, n);
        decodeURL(m_paramValue, v);
        m_pRequest->addParameter(n, v);
    }
    m_paramName.clear();
    m_paramValue.clear();
}

bool HTTPRequestParser::closePartFile() {
    bool ret = true;
    if(m_mFile) {
        if(m_mFileWriteCount) {
            std::size_t written = std::fwrite(m_mFileBuffer, sizeof(char), m_mFileWriteCount, m_mFile);
            ret = (written == m_mFileWriteCount);
            if(!ret) {
                LOG(agrade::util::Log::ERROR) << "Cannot write into temporary file :"
                                              << m_mFileName << ". Written only "
                                              << written << " bytes from "
                                              << m_mFileWriteCount << " bytes.";
            }
        }
        std::fclose(m_mFile);
        m_mFile = NULL;
    }
    m_mFileName.clear();
    m_mFileWriteCount = 0;
    return ret;
}

bool HTTPRequestParser::writeIntoPartFile(char c) {
    bool ret = true;
    if(!m_mFileBuffer) {
        m_mFileBuffer = new char[m_bufferSize];
    }
    if(!m_mFile) {
        m_mFileName = generatePartFileName();
        m_mFile = std::fopen(m_mFileName.c_str(), "wb+");
    }
    if(!m_mFile) {
        LOG(agrade::util::Log::ERROR) << "Cannot open temporary file :" << m_mFileName;
        return false;
    }
    m_mFileBuffer[m_mFileWriteCount] = c;
    m_mFileWriteCount++;
    if(m_mFileWriteCount == m_bufferSize) {
        std::size_t written = std::fwrite(m_mFileBuffer, sizeof(char), m_mFileWriteCount, m_mFile);
        ret = (written == m_mFileWriteCount);
        if(!ret) {
            LOG(agrade::util::Log::ERROR) << "Cannot write into temporary file :"
                                          << m_mFileName << ". Written only "
                                          << written << " bytes from "
                                          << m_mFileWriteCount << " bytes.";
        }
        m_mFileWriteCount = 0;
    }
    return ret;
}

std::string HTTPRequestParser::generatePartFileName() {
    boost::filesystem::path tmpDirP(m_tmpDir);
    std::ostringstream str;
    str << this << "-" << m_mFileCounter;
    ++m_mFileCounter;
    boost::filesystem::path f = tmpDirP / ("ystmp-" + str.str());
#if BOOST_FILESYSTEM_VERSION >= 3
    f = boost::filesystem::absolute(f);
#else
    f = boost::filesystem::complete(f);
#endif    
    return f.string();
}

} } } /*namespace */
