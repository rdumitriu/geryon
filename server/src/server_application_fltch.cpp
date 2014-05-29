
#include "server_application_fltch.hpp"
#include "server_application_utils.hpp"

namespace geryon { namespace server {


void ServerApplicationFilterChain::init(std::shared_ptr<geryon::Application> app) {
    std::vector<std::shared_ptr<Filter>> rawFilters = app->getFilters();
    for(std::vector<std::shared_ptr<Filter>>::iterator p = rawFilters.begin(); p != rawFilters.end(); ++p) {
        detail::WrappedFilter wf;
        wf.filter = *p;
        wf.mappedPath = detail::calculatePathFromModule(wf.filter->getApplicationModule(), wf.filter->getPath());
        wf.isRegex = false;
        if(wf.mappedPath.find("*") != std::string::npos) {
            wf.isRegex = true;
            wf.regex = std::regex(detail::createRegexFromPath(wf.mappedPath));
        }
        filters.push_back(std::move(wf));
    }
}

bool ServerApplicationFilterChain::doFilters(HttpServerRequest & request,
                                             HttpServerResponse & response) throw(geryon::HttpException) {
    for(auto wf : filters) {
        if(!doFilter(wf, request, response)) { return false; }
    }
    return true;
}

bool ServerApplicationFilterChain::doFilter(detail::WrappedFilter & flt,
                                            HttpServerRequest & request,
                                            HttpServerResponse & response) throw(geryon::HttpException) {
    if(requestMatches(flt, request)) {
        return flt.filter->doFilter(request, response);
    }
    return true;
}

bool ServerApplicationFilterChain::requestMatches(detail::WrappedFilter & flt,
                                                  HttpServerRequest & request) {
    return flt.isRegex
            ? std::regex_search(request.getURIPath(), flt.regex)
            : (request.getURIPath() == flt.mappedPath);
}


} }
