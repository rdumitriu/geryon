/*
 * HTTPProtocolHandler.cpp
 *
 *  Created on: Aug 25, 2011
 *      Author: rdumitriu
 */

#include <cstring>
#include <sstream>

#include "http_protocol_handler.hpp"

#include "log.hpp"

namespace geryon { namespace server {


HTTPProtocolHandler::HTTPProtocolHandler(TCPServerConfig * pServerConfig)
                    : TCPProtocolHandler(),
                      m_pServerConfig(pServerConfig),
                      m_Request(),
                      m_Reply(),
                      m_parser(pServerConfig->maximalContentLength(),
                               pServerConfig->getFileIOBufferSize(),
                               pServerConfig->temporaryDirectory()),
                      m_pServlet(NULL),
                      m_chunkedDelimiterLine(""),
                      m_chunkSize(0L),
                      m_totalChunkedSize(0L),
                      m_chunkTransferredSz(0L),
                      m_chunkedTransfer(false),
                      m_chunkedState(0) {
}

///Destructor
HTTPProtocolHandler::~HTTPProtocolHandler() {
}

const TCPServerConfig * HTTPProtocolHandler::getServerConfig() const {
    return m_pServerConfig;
}

void HTTPProtocolHandler::init(agrade::yahsrv::TCPInputStream & inputStream,
                               agrade::yahsrv::TCPOutputStream & outputStream) {
    LOG(agrade::util::Log::DEBUG) << "HTTP Protocol Handler init.";
    m_Request.reset(&inputStream);
    m_Reply.reset(&outputStream);
    m_parser.reset(&m_Request);
    m_pServlet = NULL;
    m_chunkedDelimiterLine.clear();
    m_chunkSize = 0L;
    m_totalChunkedSize = 0L;
    m_chunkTransferredSz = 0L;
    m_chunkedTransfer = false;
    m_chunkedState = 0;
}

void HTTPProtocolHandler::acceptRead(agrade::yahsrv::TCPInputStream & inputStream,
                                     agrade::yahsrv::TCPOutputStream & outputStream) {
    try {
        //to support additional protocols over HTTP:
        if(m_pServlet) {
            LOG(agrade::util::Log::DEBUG) << "Multi-call (http extension) on servlet:" << m_pServlet->getPath();
            m_pServlet->execute(m_Request, m_Reply);
            return;
        }
        //we're still parsing the HTTP header / request
        char c;
        bool done = false;
        agrade::yahsrv::HTTPReply::HTTPStatusCode statusCode = agrade::yahsrv::HTTPReply::SC_OK;
        while(!done) {
            inputStream >> c;
            if(m_chunkedState) {
                done = parseChunkedTESize(c, statusCode);
                if(!m_chunkedState && !done) {
                    send100Continue(outputStream);
                }
            } else {
                done = m_parser.consume(c, statusCode);
                if(!done) {
                    if(statusCode == agrade::yahsrv::HTTPReply::SC_CONTINUE) {
                        m_chunkTransferredSz = 0L;
                        send100Continue(outputStream);
                        statusCode = agrade::yahsrv::HTTPReply::SC_OK;
                        if("" == m_Request.getHeaderValue("Transfer-Encoding")) {
                            //not really a chunked transfer
                            m_chunkedTransfer = false;
                            m_chunkedState = 0;
                        } else {
                            //a trully one, parser only accept 'chunked' at this stage
                            m_chunkedTransfer = true;
                            m_chunkedState = 1;
                        }
                        #ifdef __AGRADE_DEBUG__
                        LOG(agrade::util::Log::DEBUG) << "Initiated chunked transfer, chunk transfer state:" << m_chunkedState
                                                      << ". initial reported length :" << m_Request.m_contentLength;
                        #endif
                    } else if(m_chunkedTransfer) {
                        m_chunkTransferredSz++;
                        if(m_chunkSize > 0 && m_chunkTransferredSz == m_chunkSize) {
                            m_chunkedState = 1;
                            #ifdef __AGRADE_DEBUG__
                            LOG(agrade::util::Log::DEBUG) << "Transferred chunk of size " << m_chunkTransferredSz;
                            #endif
                            m_chunkTransferredSz = 0L;
                        }
                    }
                }
            }
            if(done) {
                #ifdef __AGRADE_DEBUG__
                LOG(agrade::util::Log::DEBUG) << "Parser done, serving:" << m_Request.getURI();
                #endif
                //Connection: close
                m_Reply.addHeader("Connection", "close");
                if(m_pServerConfig->defaultCacheHeader() != "") {
                    //Cache control:
                    m_Reply.addHeader("Cache-Control", m_pServerConfig->defaultCacheHeader());
                }
                //Server header
                if(m_pServerConfig->sendServerToken()) {
                    m_Reply.addHeader("Server", m_pServerConfig->serverToken());
                }
                if(statusCode != agrade::yahsrv::HTTPReply::SC_OK) {
                    m_Reply.setStatus(statusCode);
                    outputErrorString("Error parsing header ?!?");
                    m_Reply.getOutputStream().flush();
                    break;
                }
                //HERE : We dispatch it to the proper application handler
                std::string firstPathSegment = extractFirstPathSegment(m_Request.getURIPath());

                #ifdef __AGRADE_DEBUG__
                LOG(agrade::util::Log::DEBUG) << "URI      ::" << m_Request.getURI();
                LOG(agrade::util::Log::DEBUG) << "URI Host ::" << m_Request.getURIHost();
                LOG(agrade::util::Log::DEBUG) << "URI Port ::" << m_Request.getURIPort();
                LOG(agrade::util::Log::DEBUG) << "URI Path ::" << m_Request.getURIPath();
                LOG(agrade::util::Log::DEBUG) << "URI QS   ::" << m_Request.getQueryString();
                LOG(agrade::util::Log::DEBUG) << "URI FPS  ::" << firstPathSegment;
                for(std::map<std::string, HTTPHeader>::iterator k = m_Request.m_headers.begin(); k != m_Request.m_headers.end(); ++k) {
                    LOG(agrade::util::Log::DEBUG) << "HDR      ::" << k->first
                                                  << "; value:" << m_Request.getHeaderValue(k->first);
                }
                #endif

                agrade::yahsrv::ServerApplication * pSApp = ServerApplicationRegistry::getInstance()->getApplication(firstPathSegment);
                if(pSApp) {
                    //execute the request
                    m_pServlet = pSApp->execute(m_Request, m_Reply);
                } else {
                    pSApp = ServerApplicationRegistry::getInstance()->getDefaultApplication();
                    if(pSApp) {
                        //execute it on default app, if any
                        m_pServlet = pSApp->execute(m_Request, m_Reply);
                    }
                }
                LOG(agrade::util::Log::DEBUG) << "Finished execution for path:"
                                              << m_Request.getURI() << ". Status:" << m_Reply.getStatus();
                if(!m_pServlet && m_Reply.getStatus() == HTTPReply::SC_OK) {
                    m_Reply.setStatus(HTTPReply::SC_NOT_FOUND);
                    std::string msg = "Your path '" + m_Request.getURIPath() + "' is not mapped on any application on this server (or empty response).";
                    outputErrorString(msg);
                } else {
                    //send the headers if not already sent
                    m_Reply.sendHeaders();
                }
                //flush the output stream
                try {
                    LOG(agrade::util::Log::DEBUG) << "Protocol handler almost done: Flushing the output stream.";
                    outputStream.flush();
                } catch ( ... ) {}
                //HERE
            }
        }
    } catch(TCPStreamNotReadyException & e) {
        //this signals the temporary end of the stream. Programmers should know
        //about it when reading data and act accordingly.
    } catch(MessageException & e) {
        //this is for sure a programming error
        LOG(agrade::util::Log::ERROR) << "Program exception serving resource:"
                                      << m_Request.getURI()
                                      << ". Error was:" << e.what();
    } catch( ... ) {
        LOG(agrade::util::Log::ERROR) << "Exception while serving resource:"
                                      << m_Request.getURI() << ".";
    }
}

void HTTPProtocolHandler::acceptWrite(agrade::yahsrv::TCPInputStream & inputStream, agrade::yahsrv::TCPOutputStream & outputStream) {
    //we do nothing on write
}

void HTTPProtocolHandler::done()  {
    LOG(agrade::util::Log::DEBUG) << "HTTP Protocol Handler done.";
}

std::string HTTPProtocolHandler::extractFirstPathSegment(const std::string & path) {
    if(path.length() <= 1) {
        return "/";
    }
    std::size_t pos = path.find('/', 1);
    if(pos == std::string::npos) {
        return path + "/"; //does not end with a slash, append it or full app req
    }
    return path.substr(0, pos+1);
}

void HTTPProtocolHandler::outputErrorString(const std::string & errorMsg) {
    m_Reply.setContentType("text/plain");
    std::ostringstream os;
    os << "HTTP Code: " << m_Reply.getStatus();
    if(errorMsg != "") {
        os << " - " << errorMsg;
    }
    std::string msg = os.str();
    m_Reply.setContentLength(msg.length());
    m_Reply.getOutputStream() << msg;
}

void HTTPProtocolHandler::send100Continue(agrade::yahsrv::TCPOutputStream & outputStream) {
    outputStream.write("HTTP/1.1 100 Continue\r\n\r\n", 25);
    outputStream.flush();
}

bool HTTPProtocolHandler::parseChunkedTESize(char c, agrade::yahsrv::HTTPReply::HTTPStatusCode & statusCode) {
    //chunked transfer encoding, ignore first \r\n
    if(c == '\n' && m_chunkedDelimiterLine.size()) {
        std::istringstream sdelta(m_chunkedDelimiterLine);
        sdelta >> std::hex >> m_chunkSize;
        if(m_chunkSize != 0) {
            m_totalChunkedSize += m_chunkSize;
            //revalidate the maximal content length
            if(m_totalChunkedSize > m_pServerConfig->maximalContentLength()) {
                LOG(agrade::util::Log::ERROR) << "Chunked transfer. Request too large; read so far:" << m_totalChunkedSize;
                statusCode = agrade::yahsrv::HTTPReply::SC_REQUEST_ENTITY_TOO_LARGE;
                return true;
            }
        } else {
            m_Request.m_contentLength = m_totalChunkedSize;
            m_chunkedState = 0;
            return true; //force end of parsing
        }
        #ifdef __AGRADE_DEBUG__
        LOG(agrade::util::Log::DEBUG) << "Chunked transfer, chunk length:" << m_chunkSize
                                      << " bytes, total so far:" << m_totalChunkedSize
                                      << " bytes";
        #endif
        //and clear this status back:
        m_chunkedState = 0;
        m_chunkedDelimiterLine.clear();
        return false;
    }
    if(c != '\r' && c != '\n') {
        m_chunkedDelimiterLine.push_back(c);
        #ifdef __AGRADE_DEBUG__
        LOG(agrade::util::Log::DEBUG) << "Chunk delimiter line: >>" << m_chunkedDelimiterLine << "<<";
        #endif
    }
    return false;
}

} } } /*namespace*/
