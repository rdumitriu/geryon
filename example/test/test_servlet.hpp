#ifndef GERYON_TEST_SERVLET_HPP_
#define GERYON_TEST_SERVLET_HPP_

#include <servlet.hpp>

namespace geryon { namespace test {

class TestServlet : public geryon::Servlet {
public:
    TestServlet(const std::string & _path) : geryon::Servlet(_path) {}
    virtual ~TestServlet() {}

    virtual void init();

    virtual void doGet(HttpRequest & request, HttpResponse & reply);
    virtual void doPost(HttpRequest & request, HttpResponse & reply);
    virtual void doPut(HttpRequest & request, HttpResponse & reply);
    virtual void doDelete(HttpRequest & request, HttpResponse & reply);
};

} }

#endif
