
#include "application.hpp"

namespace geryon { namespace test { 

} } /* namespaces */

//This is the entry point in the application
G_MODULE_EXPORT
std::shared_ptr<geryon::Application> createApplication() {
    std::shared_ptr<geryon::Application> app = std::make_shared<geryon::Application>("test");


    return app;
}
