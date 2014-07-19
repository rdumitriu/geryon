
#include "application.hpp"
#include "helloresponse.hpp"

namespace geryon { namespace examples { namespace helloworld {

//This is the servlet
class HelloServlet : public geryon::Servlet {
public:
    //The constructor shows where the servlet will be mounted relative to the app
    HelloServlet() : geryon::Servlet("/sayhello") {
    }

    //We respond to GET and POST methods in the same way
    void doGet(geryon::HttpRequest & request,
               geryon::HttpResponse & reply) {
        //1: Get the parameters from the request, and maybe correct them
        std::string fname = request.getParameterValue("fname");
        std::string lname = request.getParameterValue("lname");
        std::string greetings;
        boost::trim(fname);
        boost::trim(lname);
        if(fname == "" && lname == "") {
            greetings = "Hello world !";
        } else {
        	greetings = "Hello " + fname + " " + lname + " !";
        }
        //2: Set the content type
        reply.setContentType(HttpResponse::CT_APPLICATIONJSON);
        //3: spit it out
        HelloResponse hresp;
        hresp.firstName() = fname;
        hresp.lastName() = lname;
        hresp.greetings() = greetings;
        reply.getOutputStream() << hresp;
    }

    void doPost(geryon::HttpRequest & request,
                geryon::HttpResponse & reply) {
        doGet(request, reply);
    }
};

class SQLServlet : public geryon::Servlet {
public:
    SQLServlet() : geryon::Servlet("/sql-demo-1") {}
    virtual ~SQLServlet() {}

    void doGet(geryon::HttpRequest & request,
               geryon::HttpResponse & reply) {
        geryon::sql::postgres::PostgresConnection pconn_wrp(getModuleConfig().getPostgresPool("psqltest"));
        pqxx::nontransaction uow(pconn_wrp.connection());
        pqxx::result r = uow.exec("SELECT a.val, a.vdesc, b.val, b.vdesc FROM awdemonmb a, awdemonmb b");

        reply.setContentType(HttpResponse::CT_TEXT);
        for(std::size_t i = 0; i < r.size(); ++i) {
            unsigned long n1;
            unsigned long n2;
            r[i][0].to(n1);
            std::string d1(r[i][1].c_str());
            r[i][2].to(n2);
            std::string d2(r[i][3].c_str());
            reply.getOutputStream() << n1 << " - " << d1 << " -- " << n2 << " - " << d2 << "\n";
        }
    }

    void doPost(geryon::HttpRequest & request,
                geryon::HttpResponse & reply) {
        doGet(request, reply);
    }
};

} } } /* namespaces */

//This is the entry point in the application
G_MODULE_EXPORT
std::shared_ptr<geryon::Application> createApplication() {
    std::shared_ptr<geryon::Application> app = std::make_shared<geryon::Application>("demos");

    app->addServlet(std::make_shared<geryon::examples::helloworld::HelloServlet>());
    app->addServlet(std::make_shared<geryon::examples::helloworld::SQLServlet>());

    return app;
}
