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

#include "http_types.hpp"
 
namespace geryon { namespace server {

class HTTPRequestParser {
public:
    /// Constructor
    HTTPRequestParser(std::size_t maximalCT, 
                      std::size_t fileIOBufferSize, 
                      const std::string & tempDir);
    
    ///Destructor
    ~HTTPRequestParser();

    /// Reset to initial parser state.
    void reset(geryon::HTTPRequest *pRequest);
    
    ///consume a char from input
    bool consume(char c, geryon::HttpResponse::HttpStatusCode & error);
    
    ///Call this after you parsed the message
    agrade::yahsrv::HTTPReply::HTTPStatusCode validate();

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
    
    std::size_t m_maxContentLength;
    std::size_t m_bufferSize;
    std::string m_tmpDir;
    
    State m_state;
    URIState m_uriState;
    geryon::HttpRequest *m_pRequest;
    std::size_t m_postParamsCharCount;
    
    std::string m_headerName;
    std::string m_headerValue;
    
    bool consumeChar(char expected, char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error, State nextState);
    bool consumeStart(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumeMethod(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumeURI(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumeHttpVersion(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumeNewline(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error, State nextState);
    bool consumeHeaderName(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumeHeaderLWS(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumeHeaderValue(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumePostParameters(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    
    bool consumePostMultipartBoundary(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumePostMultipartNewline(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error, State nextState);
    bool consumePostMultipartHeaderOrContent(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumePostMultipartHeaderLine(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumePostMultipartContent(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumePostMultipartNextPartOrEnd(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    bool consumePostMultipartEnd(char input, agrade::yahsrv::HTTPReply::HTTPStatusCode & error);
    
    //URI_break_down
    std::string m_paramName;
    std::string m_paramValue;
    bool consumeURISubstate(char c);
    void pushRequestParameter();
    bool validateMethod();
    
    //Multipart:
    std::string m_multipartSeparator;
    std::size_t m_multipartSeparatorIndex;
    std::string m_multipartHeaderLine;
    std::string m_multipartFileName;
    std::string m_multipartCountentType;
    std::string m_multipartCountentEncoding;
    std::string m_mFileName;
    std::FILE * m_mFile;
    std::size_t m_mFileWriteCount;
    char * m_mFileBuffer;
    unsigned int m_mFileCounter;
    
    std::deque<char> m_multipartContentQueue;
    bool extractMultipartSeparator();
    bool multipartContentMatchesBoundary();
    void countPostBytes();
    bool processMultipartHeader();
    void parseContentDispositionHeader();
    
    std::string generatePartFileName();
    bool writeIntoPartFile(char c);
    bool closePartFile();
};

} } /* namespace */

#endif
