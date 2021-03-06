
#include "server_application_fltch.hpp"

#include "log.hpp"

namespace geryon { namespace server {


void ServerApplicationFilterChain::init(std::shared_ptr<geryon::Application> & app) {
    LOG(geryon::util::Log::DEBUG) << "Calculating filters for app :" << app->getKey();
    std::vector<std::shared_ptr<Filter>> rawFilters;
    app->getFilters(rawFilters);
    for(std::vector<std::shared_ptr<Filter>>::iterator p = rawFilters.begin(); p != rawFilters.end(); ++p) {
        detail::WrappedFilter wf;
        wf.filter = *p;
        std::string calculatedPath = detail::calculatePathFromModule(wf.filter->getApplicationModule(),
                                                                     wf.filter->getPath());
        wf.matchEntry = detail::createMatchingEntry(calculatedPath);
        filters.push_back(std::move(wf));
    }
    for(unsigned int i = 0; i < filters.size(); ++i) {
        detail::WrappedFilter & rwf = filters.at(i);
        index.addNode(rwf.matchEntry.mappedPath, i);
    }
}

bool ServerApplicationFilterChain::doFilters(HttpServerRequest & request,
                                             HttpServerResponse & response) {
    std::vector<unsigned int> filterSet = index.getDataForPath(request.getURIPath());
    //the filter set is ordered from root to most specific
    //although may not be sane, we consider it a good rule to execute the filters

    // Note: we keep only indexes here, but we could easily store the entire filter structure
    // However, the overhead is minimal (an indexed call for each matched filter)
    for(unsigned int ndx : filterSet) {
        detail::WrappedFilter & wf = filters.at(ndx);
        if(!doFilter(wf, request, response)) { return false; }
    }
    return true;
}

bool ServerApplicationFilterChain::doFilter(const detail::WrappedFilter & flt,
                                            HttpServerRequest & request,
                                            HttpServerResponse & response) {

    if(detail::isMatchingEntry(request.getURIPath(), flt.matchEntry)) {
        return flt.filter->doFilter(request, response);
    }
    return true;
}

} }
