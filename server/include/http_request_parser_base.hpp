///
/// \file http_request_parser_base.hpp
///
///  Created on: Aug 24, 2014
///      Author: rdumitriu at gmail.com

#ifndef GERYON_HTTPREQUEST_PARSER_BASE_HPP_
#define GERYON_HTTPREQUEST_PARSER_BASE_HPP_

#include "http_server_types.hpp"

namespace geryon { namespace server {

/// Never instantiate me
class AbstractHttpRequestParserBase {
public:
    AbstractHttpRequestParserBase(std::size_t maximalContentLenght, std::size_t & absoluteIndex);
    ///Destructor
    virtual ~AbstractHttpRequestParserBase();

    enum ParserAction {
        PA_DONE,
        PA_CONTINUE,
        PA_CHECK_HEADERS,
        PA_CONTINUEACTION
    };

    /// Reset to initial parser state.
    virtual void init(geryon::server::HttpServerRequest *pRequest);

    ///consume a char from input
    virtual ParserAction consume(char c, geryon::HttpResponse::HttpStatusCode & error);

    ///Report back the maximal content length
    inline std::size_t maximalContentLenght() const { return maxContentLength; }

    ///Report the absolute index (incremented outside!)
    inline std::size_t getAbsoluteIndex() { return absoluteIndex; }

protected:
    std::size_t maxContentLength;
    std::size_t & absoluteIndex;
    geryon::server::HttpServerRequest *pRequest;

};

} }

#endif
