///
/// \file server_application_fltch.hpp
///
///  Created on: May 23, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_SERVERAPPLICATIONFLTCH_HPP_
#define GERYON_SERVERAPPLICATIONFLTCH_HPP_

#include <vector>
#include <regex>

#include "application.hpp"
#include "http_server_types.hpp"
#include "path_tree.hpp"
#include "server_application_utils.hpp"

namespace geryon { namespace server {

namespace detail {

    struct WrappedFilter {
        std::shared_ptr<Filter> filter;
        MatchingEntry matchEntry;
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

    bool doFilters(HttpServerRequest & request, HttpServerResponse & response);
private:
    std::vector<detail::WrappedFilter> filters;
    PathTree<unsigned int> index;

    bool doFilter(const detail::WrappedFilter & flt,
                  HttpServerRequest & request, HttpServerResponse & response);
};

} }

#endif
