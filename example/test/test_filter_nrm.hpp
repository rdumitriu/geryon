#ifndef GERYON_TEST_FILTER_NRM_HPP_
#define GERYON_TEST_FILTER_NRM_HPP_

#include <filter.hpp>

namespace geryon { namespace test {

//No Response (hijacked)
class TestFilterNrm : public geryon::Filter {
public:
    TestFilterNrm(const std::string & _path) : Filter(_path) {}
    virtual ~TestFilterNrm() {}

    virtual bool doFilter(geryon::HttpRequest & request, geryon::HttpResponse & reply);
};

} }

#endif
