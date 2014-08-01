
#include "test_filter_global.hpp"

namespace geryon { namespace test {

//virtual
bool TestFilterGlobal::doFilter(geryon::HttpRequest & request, geryon::HttpResponse & reply) {
    request.putAttribute("global.filter.attr", std::string("GFT.PASSED"));
    return true;
}

} } /* namespace */
