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
#include "server_application_fltch.hpp"
#include "server_application_srvdsp.hpp"

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



class ServerApplicationServletFinder { //::TODO:: should be moved out? returns a single servlet
};

}

class ServerApplication {
public:
    ServerApplication(std::shared_ptr<geryon::Application> application,
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
    ServerApplicationFilterChain filterChain;
    //servlets mappings:
    ServerApplicationServletDispatcher servletDispatcher;


    void prepareExecution(HttpServerRequest & request, HttpServerResponse & response) throw(geryon::HttpException);

    std::string getSessionCookieValue(HttpServerRequest & request);
    std::size_t getPartitionFromCookie(const std::string & cookie);
    std::shared_ptr<ServerSession> getSession(const std::string & cookie);
    std::shared_ptr<ServerSession> createSession(HttpServerRequest & request, HttpServerResponse & response);
};

} } /*namespace */

#endif
