
#include "test_filter_nrm.hpp"
#include "filterresponse.hpp"

namespace geryon { namespace test {

//virtual
bool TestFilterNrm::doFilter(geryon::HttpRequest & request, geryon::HttpResponse & reply) {
    reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
    FilterResponse resp;
    resp.message() = "NRF.PASSED";
    reply.getOutputStream() << resp;
    return false;
}

} } /* namespace */
