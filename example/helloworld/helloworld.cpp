
#include "application.hpp"

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
        reply.setContentType("text/plain");
        //3: Serialize it
        reply.getOutputStream() << greetings;
    }

    void doPost(geryon::HttpRequest & request,
                geryon::HttpResponse & reply) {
        doGet(request, reply);
    }
};

} } } /* namespaces */

//This is the entry point in the application
extern "C"
std::shared_ptr<geryon::Application> createApplication() {
    std::shared_ptr<geryon::Application> app = std::make_shared<geryon::Application>("demos.helloword");

    app->addServlet(std::make_shared<geryon::examples::helloworld::HelloServlet>());

    return app;
}
