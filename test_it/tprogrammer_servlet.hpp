#ifndef GERYON_TEST_PROGRAMMER_SERVLET_HPP_
#define GERYON_TEST_PROGRAMMER_SERVLET_HPP_

#include <servlet.hpp>

namespace geryon { namespace test {

///
/// \brief The TestProgrammerServlet class
///
/// This is my first test for backbone.js. Pretty much works.
///
class TestProgrammerServlet : public geryon::Servlet {
public:
    TestProgrammerServlet(const std::string & _path) : geryon::Servlet(_path) {}
    virtual ~TestProgrammerServlet() {}

    virtual void init() {}

    virtual void doGet(HttpRequest & request, HttpResponse & reply);
    virtual void doPost(HttpRequest & request, HttpResponse & reply);
    virtual void doPut(HttpRequest & request, HttpResponse & reply);
    virtual void doDelete(HttpRequest & request, HttpResponse & reply);
};

} }

#endif
