///
/// \file server_application.hpp
///
///  Created on: May 23, 2014
///      Author: rdumitriu at gmail.com
///
#ifndef SERVERAPPLICATION_HPP_
#define SERVERAPPLICATION_HPP_

#include <map>
#include <vector>

#include "application.hpp"
#include "repetitive_runnable.hpp"
#include "http_server_types.hpp"

#include "server_session.hpp"

namespace geryon { namespace server {

namespace detail {

class SessionPartition {
public:

    ///Gets an existing session
    std::shared_ptr<ServerSession> getSession(const std::string & cookie);
    ///Registers a session
    void registerSession(const std::string & cookie, std::shared_ptr<ServerSession> ses);

    void cleanup(unsigned int sessTimeOut);

private:
    std::map<std::string, std::shared_ptr<ServerSession>> sessions;
    ///Sessions:: the sessions protector mutex, shared
    std::mutex smutex;
};

class FilterMatcher {

};

class ServletMatcher {
};

}

class ServerApplication {
public:
    ServerApplication(std::shared_ptr<geryon::Application> application,
                      const std::string & serverToken,
                      unsigned int serverNumber,
                      unsigned int nPartitions,
                      unsigned int sessTimeOut,
                      unsigned int cleanupInterval);
    ~ServerApplication();

    ServerApplication(const ServerApplication & other) = delete;
    ServerApplication & operator = (const ServerApplication & other) = delete;

    void start();
    void stop();

    inline const std::string & getPath() { return path; }

    void execute(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);
private:
    std::shared_ptr<geryon::Application> application;
    std::string path;
    //serverToken
    std::string serverToken;
    //sessions:
    unsigned int serverNumber;
    unsigned int nPartitions;
    unsigned int sessTimeOut;
    detail::SessionPartition * sessionPartitions;
    geryon::mt::RepetitiveRunnable sessionCleaner;
    //filter mappings:
    std::map<std::string, std::shared_ptr<detail::FilterMatcher>> filters;
    //servlets mappings:
    std::map<std::string, std::shared_ptr<detail::ServletMatcher>> servlets;


    void prepareExecution(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);
    bool runFilters(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);
    bool runServlet(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);

    std::string getSessionCookieValue(HttpServerRequest & request);
    std::size_t getPartitionFromCookie(const std::string & cookie);
    std::shared_ptr<ServerSession> getSession(const std::string & cookie);
    std::shared_ptr<ServerSession> createSession(HttpServerRequest & request, HttpServerResponse & response);
};

} } /*namespace */

#endif
