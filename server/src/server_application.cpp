
#include "string_utils.hpp"
#include "server_application.hpp"
#include "server_global_structs.hpp"

#include "log.hpp"

namespace geryon { namespace server {

namespace detail {
/* =================================================================================
 * S E S S I O N  P A R T I T I O N S
 * ================================================================================= */

std::shared_ptr<ServerSession> SessionPartition::getSession(const std::string & cookie) {
    std::lock_guard<std::mutex> _(smutex);
    std::map<std::string, std::shared_ptr<ServerSession>>::iterator p = sessions.find(cookie);
    std::shared_ptr<ServerSession> ret;
    if(p != sessions.end()) {
        ret = p->second;
        ret->updateTimeStamp();
    }
    return ret;
}

void SessionPartition::registerSession(const std::string & cookie, std::shared_ptr<ServerSession> ses) {
    std::lock_guard<std::mutex> _(smutex);
    sessions.insert(std::make_pair(cookie, ses));
}

void SessionPartition::cleanup(unsigned int timeout) {
    using namespace std::chrono;
    seconds tm(timeout);
    system_clock::time_point now = system_clock::now();

    std::lock_guard<std::mutex> _(smutex);
    LOG(geryon::util::Log::DEBUG) << "Total sessions in the partition :" << sessions.size();

    for(std::map<std::string, std::shared_ptr<ServerSession>>::iterator p = sessions.begin(); p != sessions.end(); ++p) {
        system_clock::time_point sesTS = system_clock::from_time_t(p->second->getTimeStamp());
        seconds elapsed = seconds(duration_cast<seconds>(now - sesTS));
        if(elapsed.count() > tm.count()) {
            LOG(geryon::util::Log::DEBUG) << "Watchdog thread is destroying session :" << p->first;
            sessions.erase(p->first); //you're dead and gone, sweetheart
        }
    }
}

SessionStats SessionPartition::getStats() {
    SessionStats ret;
    std::lock_guard<std::mutex> _(smutex);
    ret.count = sessions.size();
    ret.totalSize = 0;
    for(std::map<std::string, std::shared_ptr<ServerSession>>::iterator p = sessions.begin(); p != sessions.end(); ++p) {
        ret.totalSize += sizeof(*(p->second));
    }
    return ret;
}


} /*namespace detail*/

/* =================================================================================
 * S E R V E R  A P P L I C A T I O N
 * ================================================================================= */

void cleanupServerApplicationSessions(unsigned int sessTimeOut, unsigned int nPartitions, detail::SessionPartition *partitions) {
    for(unsigned int i = 0; i < nPartitions; ++i) {
        partitions[i].cleanup(sessTimeOut);
    }
}

ServerApplication::ServerApplication(std::shared_ptr<geryon::Application> & _application,
                                     unsigned int _nPartitions,
                                     unsigned int _sessTimeOut,
                                     unsigned int cleanupInterval)
                    : application(_application),
                      path(_application->getConfig().getMountPath()),
                      serverToken(ServerGlobalStructs::getServerToken()),
                      serverNumber(ServerGlobalStructs::getServerId()),
                      nPartitions(_nPartitions),
                      sessTimeOut(_sessTimeOut),
                      sessionPartitions(new detail::SessionPartition[_nPartitions]),
                      sessionCleaner(cleanupInterval, cleanupServerApplicationSessions, sessTimeOut, nPartitions, sessionPartitions) {
    if(!geryon::util::endsWith(path, "/")) {
        path += "/";
    }
}


ServerApplication::~ServerApplication() {
    delete [] sessionPartitions;
}

std::vector<detail::SessionStats> ServerApplication::getStats() {
    std::vector<detail::SessionStats> ret;
    for(unsigned int i = 0; i < nPartitions; ++i) {
        ret.push_back(std::move(sessionPartitions[i].getStats()));
    }
    return ret;
}

void ServerApplication::start() {
    LOG(geryon::util::Log::INFO) << "Starting application " << application->getKey() << " ...";
    //1: build filters
    filterChain.init(application);
    //2: build servlets
    servletDispatcher.init(application);
    //3: start the session cleaner
    sessionCleaner.start();
    //4:: start the app
    application->start();
    LOG(geryon::util::Log::INFO) << "Started application " << application->getKey() << ".";
}

void ServerApplication::stop() {
    sessionCleaner.stop();
    application->stop();
}

void ServerApplication::execute(HttpServerRequest & request, HttpServerResponse & response) {
    try {
        prepareExecution(request, response);
        if(filterChain.doFilters(request, response)) {
            if(!servletDispatcher.doServlet(request, response)) {
                std::string msg = "Cannot find servlet or filter to deal with: " + request.getURI();
                sendStandardResponse(response, HttpResponse::SC_NOT_FOUND, msg);
                LOG(geryon::util::Log::ERROR) << msg;
            }
        }
    } catch(geryon::HttpException & e) {
        std::string msg("Internal error (HTTP):");
        msg += e.what();
        sendStandardResponse(response, HttpResponse::SC_INTERNAL_SERVER_ERROR, msg);
        LOG(geryon::util::Log::ERROR) << msg;
    } catch(std::runtime_error & e) {
        std::string msg("Internal error (RUNTIME):");
        msg += e.what();
        sendStandardResponse(response, HttpResponse::SC_INTERNAL_SERVER_ERROR, msg);
        LOG(geryon::util::Log::ERROR) << msg;
    } catch( ... ) {
        std::string msg("Internal error: unspecified, generic.");
        sendStandardResponse(response, HttpResponse::SC_INTERNAL_SERVER_ERROR, msg);
        LOG(geryon::util::Log::ERROR) << msg;
    }
    response.flush();
    response.close(); //::TODO:: keep-alive should change that
}

void ServerApplication::sendStandardResponse(HttpServerResponse & response,
                                             HttpResponse::HttpStatusCode code,
                                             const std::string & msg) {
    //this may throw away if the response is already commited.
    response.setStatus(code);
    response.setContentType(HttpResponse::CT_TEXT);
    response.getOutputStream() << msg;
}

void ServerApplication::prepareExecution(HttpServerRequest & request,
                                         HttpServerResponse & response) {
    //new request, session not set, we should do it
    std::string sessionCookieValue = getSessionCookieValue(request);

    std::shared_ptr<Session> pSession;
    if(sessionCookieValue != "") {
        LOG(geryon::util::Log::DEBUG) << "Session cookie found:" << sessionCookieValue;
        //session should be in the map, most probably
        pSession = getSession(sessionCookieValue);
    }
    if(!pSession.get()) {
        pSession = createSession(request, response);
    }
    //until now, the request has only the cookie, but not the session itself, so:
    request.setSession(pSession);

    //fix the response accordingly
    response.addHeader("Connection", "close"); //::TODO:: keep-alive should change that
    if("" != serverToken) {
        response.addHeader("Server", serverToken);
    }
}

const char * const SESSION_PREFIX = "geryonsessid="; //exactly 13 chars
const char * const SET_COOKIE_HEADER = "Set-Cookie";
const char * const COOKIE_HEADER = "Cookie";

std::string ServerApplication::getSessionCookieValue(HttpServerRequest & request) {
    if(request.getSessionCookie() != "") {
        return request.getSessionCookie();
    }
    geryon::HttpCookie sessCookie;
    if(request.getCookie("geryonsessid", sessCookie)) {
        request.setSessionCookie(sessCookie.value);
        return sessCookie.value;
    }
    return "";
}

std::size_t ServerApplication::getPartitionFromCookie(const std::string & cookie) {
    char * p = const_cast<char *>(cookie.c_str());
    unsigned int sum = 0;
    for(int i = 0 ; i < 8 && *p; ++i) {
        sum += *p;
        p++;
    }
    std::size_t partNo = (sum % nPartitions);
    LOG(geryon::util::Log::DEBUG) << "Partition for cookie " << cookie << " is " << partNo;
    return partNo;
}

std::shared_ptr<ServerSession> ServerApplication::getSession(const std::string & cookie) {
    //get from partition
    return sessionPartitions[getPartitionFromCookie(cookie)].getSession(cookie);
}

std::shared_ptr<ServerSession> ServerApplication::createSession(HttpServerRequest & request, HttpServerResponse & reply) {
    std::shared_ptr<ServerSession> pSession = std::make_shared<ServerSession>(this->application.get());

    std::ostringstream os;
    os << pSession.get() << "." << serverNumber; //address '.' serverNum
    // that's what is sent back by the browser
    std::string cookie = os.str();
    request.setSessionCookie(cookie);

    //Session cookie
    std::ostringstream headeros;
    headeros << SESSION_PREFIX << cookie << "; " << "Path=" << getPath() << "; HttpOnly";
    std::string cookieValue = headeros.str();
    LOG(geryon::util::Log::DEBUG) << "Created Session cookie:" << cookie << " Value=" << cookieValue;
    reply.addHeader(SET_COOKIE_HEADER, cookieValue);

    //ROUTEID is used for load balancing
    std::ostringstream osrt;
    osrt << "ROUTEID=" << serverNumber << "; " << "Path=" << getPath() << "; HttpOnly";
    reply.addHeader(SET_COOKIE_HEADER, osrt.str());

    //set into the designated partition
    sessionPartitions[getPartitionFromCookie(cookie)].registerSession(cookie, pSession);
    return pSession;
}


} }
