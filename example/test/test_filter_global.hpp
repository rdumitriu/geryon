
#ifndef GERYON_TEST_FILTER_GLOBAL_HPP_
#define GERYON_TEST_FILTER_GLOBAL_HPP_

#include <filter.hpp>

namespace geryon { namespace test {

class TestFilterGlobal : public geryon::Filter {
public:
    TestFilterGlobal(const std::string & _path) : Filter(_path) {}
    virtual ~TestFilterGlobal() {}

    virtual bool doFilter(geryon::HttpRequest & request, geryon::HttpResponse & reply);
};

} }

#endif
