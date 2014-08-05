
#include "log.hpp"

#include "http_mt_executor.hpp"

#define QUEUE_CAPACITY_FACTOR 10

namespace geryon { namespace server {

void HttpMultiThreadExecutor_exec(const detail::HMTMessage & m) {
    m.pApp->execute(*(m.pRequest), *(m.pResponse));
}

HttpMultiThreadExecutor::HttpMultiThreadExecutor(unsigned int _nThreads)
                                : HttpExecutor(),
                                  nThreads(_nThreads),
                                  messageQueue(HttpMultiThreadExecutor_exec, _nThreads, _nThreads * QUEUE_CAPACITY_FACTOR) {
}

void HttpMultiThreadExecutor::executeInternal(std::shared_ptr<ServerApplication> papp,
                                 HttpServerRequest & request,
                                 HttpServerResponse & response) {
    messageQueue.execute(std::move(detail::HMTMessage(papp, request, response)));
}

} }
