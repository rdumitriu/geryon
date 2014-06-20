
#include "server_application_fltch.hpp"
#include "server_application_utils.hpp"

#include "log.hpp"

namespace geryon { namespace server {


void ServerApplicationFilterChain::init(std::shared_ptr<geryon::Application> & app) {
    LOG(geryon::util::Log::DEBUG) << "Calculating filters for app :" << app->getKey();
    std::vector<std::shared_ptr<Filter>> rawFilters;
    app->getFilters(rawFilters);
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
    for(unsigned int i = 0; i < filters.size(); ++i) {
        detail::WrappedFilter & rwf = filters.at(i);
        index.addNode(rwf.mappedPath, i);
    }
}

bool ServerApplicationFilterChain::doFilters(HttpServerRequest & request,
                                             HttpServerResponse & response) throw(geryon::HttpException) {
    std::vector<unsigned int> filterSet = index.getDataForPath(request.getURIPath());
    //the filter set is ordered from root to most specific
    //although may not be sane, we consider it a good rule to execute the filters
    for(unsigned int ndx : filterSet) {
        detail::WrappedFilter & wf = filters.at(ndx);
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
            ? std::regex_match(request.getURIPath(), flt.regex)
            : (request.getURIPath() == flt.mappedPath);
}


} }
