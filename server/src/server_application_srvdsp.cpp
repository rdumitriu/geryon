
#include "server_application_srvdsp.hpp"
#include "server_application_utils.hpp"

namespace geryon { namespace server {


void ServerApplicationServletDispatcher::init(std::shared_ptr<geryon::Application> & app) {
    std::vector<std::shared_ptr<Servlet>> rawServlets;
    app->getServlets(rawServlets);
    for(std::vector<std::shared_ptr<Servlet>>::iterator p = rawServlets.begin(); p != rawServlets.end(); ++p) {
        std::string path = detail::calculatePathFromModule((*p)->getApplicationModule(), (*p)->getPath());
        if(path.find("*") == std::string::npos) {
            //this is direct
            directMappedServlets.insert(std::make_pair(path, *p));
        } else {
            //regex, needs scan
            detail::WrappedServlet srvt;
            srvt.servlet = *p;
            srvt.mappedPath = path;
            srvt.regex = std::regex(detail::createRegexFromPath(path));
            regexMappedServlets.push_back(std::move(srvt));
        }
    }
    for(unsigned int i = 0; i < regexMappedServlets.size(); ++i) {
        detail::WrappedServlet & rws = regexMappedServlets.at(i);
        regexMappedIndex.addNode(rws.mappedPath, i);
    }
}

bool ServerApplicationServletDispatcher::doServlet(HttpServerRequest & request,
                                                   HttpServerResponse & response) throw(geryon::HttpException) {
    std::shared_ptr<Servlet> servlet = findServlet(request);
    if(servlet.get()) {
        servlet->execute(request, response);
        return true;
    }
    return false;
}

//
// Note: we keep it separate, we prefer the direct map for non-regex servlets (faster)
// For regex servlets, we look into the index in an attempt to avoid matching as much as possible.
// Since the tree has the most relevant match last, we use the reverse iterator on it.
//
// Note: we keep only indexes here, but we could easily store the entire servlet regex structure
// However, the overhead is minimal (an indexed call)
std::shared_ptr<Servlet> ServerApplicationServletDispatcher::findServlet(HttpServerRequest & request) {
    std::map<std::string, std::shared_ptr<Servlet>>::iterator p = directMappedServlets.find(request.getURIPath());
    if(p != directMappedServlets.end()) {
        return p->second;
    }
    std::vector<unsigned int> possMatches = regexMappedIndex.getDataForPath(request.getURIPath());
    for(std::vector<unsigned int>::reverse_iterator ndx = possMatches.rbegin(); ndx != possMatches.rend(); ++ndx) {
        detail::WrappedServlet & regsrv = regexMappedServlets.at(*ndx);
        if(requestMatches(regsrv, request)) {
            return regsrv.servlet;
        }
    }
    return std::shared_ptr<Servlet>(0);
}

bool ServerApplicationServletDispatcher::requestMatches(const detail::WrappedServlet & srv,
                                                        const HttpServerRequest & request) {
    return std::regex_match(request.getURIPath(), srv.regex);
}


} }
