///
/// \file http_mt_executor.hpp
///
///  Created on: May 22, 2014
///      Author: rdumitriu at gmail.com

#ifndef HTTPMULTITHREADEXECUTOR_HPP_
#define HTTPMULTITHREADEXECUTOR_HPP_

#include "http_executor.hpp"
#include "thread_pool.hpp"

namespace geryon { namespace server {

namespace detail {

struct HMTMessage {
    HMTMessage() : pApp(), pRequest(0), pResponse(0) {}
    HMTMessage(std::shared_ptr<ServerApplication> _pApp, HttpServerRequest & _request, HttpServerResponse & _response)
        : pApp(_pApp), pRequest(&_request), pResponse(&_response) {}
    ~HMTMessage() {}
    std::shared_ptr<ServerApplication> pApp;
    HttpServerRequest * pRequest;
    HttpServerResponse * pResponse;
};

}

class HttpMultiThreadExecutor : public HttpExecutor {
public:
    HttpMultiThreadExecutor(unsigned int _nThreads);
    virtual ~HttpMultiThreadExecutor() {}

protected:
    virtual void executeInternal(std::shared_ptr<ServerApplication> papp,
                                 HttpServerRequest & request,
                                 HttpServerResponse & response) throw(geryon::HttpException);
private:
    unsigned int nThreads;
    geryon::mt::QueuedThreadPool<detail::HMTMessage> messageQueue;
};

} }

#endif
