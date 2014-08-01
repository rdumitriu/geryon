#include "test_servlet.hpp"
#include "servletresponse.hpp"

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

//virtual
void TestServlet::init() {
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
