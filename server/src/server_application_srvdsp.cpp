
#include "server_application_srvdsp.hpp"
#include "server_application_utils.hpp"

namespace geryon { namespace server {


void ServerApplicationServletDispatcher::init(std::shared_ptr<geryon::Application> app) {
    std::vector<std::shared_ptr<Servlet>> rawServlets = app->getServlets();
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

std::shared_ptr<Servlet> ServerApplicationServletDispatcher::findServlet(HttpServerRequest & request) {
    std::map<std::string, std::shared_ptr<Servlet>>::iterator p = directMappedServlets.find(request.getURIPath());
    if(p != directMappedServlets.end()) {
        return p->second;
    }
    for(auto regsrv : regexMappedServlets) {
        if(requestMatches(regsrv, request)) {
            return regsrv.servlet;
        }
    }
    return std::shared_ptr<Servlet>(0);
}

bool ServerApplicationServletDispatcher::requestMatches(detail::WrappedServlet & srv,
                                                        HttpServerRequest & request) {
    return std::regex_match(request.getURIPath(), srv.regex);
}


} }
