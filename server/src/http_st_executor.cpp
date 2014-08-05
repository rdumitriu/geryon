
#include "http_st_executor.hpp"

namespace geryon { namespace server {

void HttpSingleThreadExecutor::executeInternal(std::shared_ptr<ServerApplication> papp,
                                 HttpServerRequest & request,
                                 HttpServerResponse & response) {
    papp->execute(request, response);
}

} }
