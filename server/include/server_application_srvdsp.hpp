///
/// \file server_application_fltch.hpp
///
///  Created on: May 23, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef GERYON_SERVERAPPLICATIONSRVDSP_HPP_
#define GERYON_SERVERAPPLICATIONSRVDSP_HPP_

#include <map>
#include <vector>
#include <regex>

#include "application.hpp"
#include "http_server_types.hpp"
#include "path_tree.hpp"
#include "server_application_utils.hpp"

namespace geryon { namespace server {

namespace detail {

    struct WrappedServlet {
        std::shared_ptr<Servlet> servlet;
        MatchingEntry matchEntry;
    };
}

///
/// \brief The ServerApplicationServletDispatcher class
///
class ServerApplicationServletDispatcher {
public:
    ServerApplicationServletDispatcher() {}
    ~ServerApplicationServletDispatcher() {}

    void init(std::shared_ptr<geryon::Application> & app);

    bool doServlet(HttpServerRequest & request, HttpServerResponse & response);
private:
    std::map<std::string, std::shared_ptr<Servlet>> directMappedServlets;
    std::vector<detail::WrappedServlet> regexMappedServlets;
    PathTree<unsigned int> regexMappedIndex;

    std::shared_ptr<Servlet> findServlet(HttpServerRequest & request);
};

} }

#endif
