#include <sstream>

#include "test_servlet.hpp"
#include "servletresponse.hpp"
#include "postrequest.hpp"

namespace geryon { namespace test {

class STest {
public:
    STest() {}
    virtual ~STest() {}
    virtual void runTest(HttpRequest & request, HttpResponse & reply) = 0;
};



STest * selectTest(const HttpRequest & request, const std::vector<std::shared_ptr<STest>> & v) {
    unsigned int testNo = request.getParameterValue("test", 0U);
    if(testNo < v.size()) {
        return v[testNo].get();
    }
    return 0;
}

void runTest(HttpRequest & request, HttpResponse & reply, const std::vector<std::shared_ptr<STest>> & v) {
    STest * pT = selectTest(request, v);
    if(!pT) {
        ServletResponse sr;
        sr.ok() = false;
        sr.code() = 1;
        sr.message() = "No test selected, or index out of range";
        reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
        reply.getOutputStream() << sr;
        return;
    }
    pT->runTest(request, reply);
}

std::vector<std::shared_ptr<STest>> GET_TESTS;
std::vector<std::shared_ptr<STest>> POST_TESTS;
std::vector<std::shared_ptr<STest>> PUT_TESTS;
std::vector<std::shared_ptr<STest>> DEL_TESTS;

// Servlet test: can get a param
class SimpleParamTest : public STest {
public:
    SimpleParamTest() : STest() {}
    virtual ~SimpleParamTest() {}

    virtual void runTest(HttpRequest & request, HttpResponse & reply) {
        ServletResponse sr;
        reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
        std::string s = request.getParameterValue("param");
        sr.code() = 0;
        sr.message() = s;
        sr.ok() = true;
        reply.getOutputStream() << sr;
    }
};


//Global filter test : filter executes
class GlobalFilterTest : public STest {
public:
    GlobalFilterTest() : STest() {}
    virtual ~GlobalFilterTest() {}

    virtual void runTest(HttpRequest & request, HttpResponse & reply) {
        ServletResponse sr;
        reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
        std::string s;
        request.getAttribute("global.filter.attr", s);
        sr.code() = 0;
        sr.message() = s;
        sr.ok() = s == "GFT.PASSED";
        reply.getOutputStream() << sr;
    }
};

class JsonRequestTest : public STest {
public:
    JsonRequestTest() : STest() {}
    virtual ~JsonRequestTest() {}

    virtual void runTest(HttpRequest & request, HttpResponse & reply) {
        PostRequest pr;
        ServletResponse sr;
        request.getInputStream() >> pr;
        reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
        sr.code() = 0;
        sr.ok() = true;
        std::ostringstream str;
        std::string s = (pr.do_something().get() ? (*pr.do_something() ? "truth" : "fake") : "inconclusive");
        str << pr.msg() << "-" << pr.other_msgs().size() << "-" << s;
        sr.message() = str.str();
        reply.getOutputStream() << sr;
    }
};

//virtual
void TestServlet::init() {
    GET_TESTS.push_back(std::make_shared<SimpleParamTest>());
    GET_TESTS.push_back(std::make_shared<GlobalFilterTest>());

    POST_TESTS.push_back(std::make_shared<SimpleParamTest>());
    POST_TESTS.push_back(std::make_shared<GlobalFilterTest>());
    POST_TESTS.push_back(std::make_shared<JsonRequestTest>());

    PUT_TESTS.push_back(std::make_shared<JsonRequestTest>());
}

//virtual
void TestServlet::doGet(HttpRequest & request, HttpResponse & reply) {
    runTest(request, reply, GET_TESTS);
}

//virtual
void TestServlet::doPost(HttpRequest & request, HttpResponse & reply) {
    runTest(request, reply, POST_TESTS);
}

//virtual
void TestServlet::doPut(HttpRequest & request, HttpResponse & reply) {
    runTest(request, reply, PUT_TESTS);
}

//virtual
void TestServlet::doDelete(HttpRequest & request, HttpResponse & reply) {
    runTest(request, reply, DEL_TESTS);
}

} }
