#include <sstream>

#include "tprogrammer.hpp"
#include "tprogrammer_servlet.hpp"

#include "log.hpp"

namespace geryon { namespace test {


//virtual
void TestProgrammerServlet::doGet(HttpRequest & request, HttpResponse & reply) {
    TProgrammer programmer;
    programmer.id() = getLastPathParam(request, 0L);
    programmer.name() = "Programarici Programarescu";
    programmer.mentalAge() = 10;
    programmer.lang() = "Java";

    reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
    reply.getOutputStream() << programmer;
}

//virtual
void TestProgrammerServlet::doPost(HttpRequest & request, HttpResponse & reply) {
    TProgrammer programmer;
    request.getInputStream() >> programmer;

    LOG(geryon::util::Log::INFO) << "POST:" << programmer;

    reply.setStatus(geryon::HttpResponse::SC_NO_CONTENT);
}

//virtual
void TestProgrammerServlet::doPut(HttpRequest & request, HttpResponse & reply) {
    TProgrammer programmer;
    request.getInputStream() >> programmer;

    LOG(geryon::util::Log::INFO) << "PUT:" << programmer;
    reply.setStatus(geryon::HttpResponse::SC_CREATED);
    reply.setContentType(HttpResponse::HttpContentType::CT_APPLICATIONJSON);
    reply.getOutputStream() << programmer;
}

//virtual
void TestProgrammerServlet::doDelete(HttpRequest & request, HttpResponse & reply) {
    LOG(geryon::util::Log::INFO) << "DELETE:" << getLastPathParam(request, 0L);
}

} }
