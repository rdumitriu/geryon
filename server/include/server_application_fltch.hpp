///
/// \file server_application_fltch.hpp
///
///  Created on: May 23, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef SERVERAPPLICATIONFLTCH_HPP_
#define SERVERAPPLICATIONFLTCH_HPP_

#include <vector>
#include <regex>

#include "application.hpp"
#include "http_server_types.hpp"

namespace geryon { namespace server {

namespace detail {

    struct WrappedFilter {
        std::shared_ptr<Filter> filter;
        std::string mappedPath;
        bool isRegex;
        std::regex regex;
    };
}

///
/// \brief The ServerApplicationFilterChain class
///
class ServerApplicationFilterChain {
public:
    ServerApplicationFilterChain() {}
    ~ServerApplicationFilterChain() {}

    void init(std::shared_ptr<geryon::Application> & app);

    bool doFilters(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);
private:
    std::vector<detail::WrappedFilter> filters;

    bool doFilter(detail::WrappedFilter & flt,
                  HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);

    bool requestMatches(detail::WrappedFilter & flt, HttpServerRequest & request);
};

} }

#endif
