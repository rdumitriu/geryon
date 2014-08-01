
#include "application.hpp"

#include "test_filter_global.hpp"
#include "test_filter_nrm.hpp"

//This is the entry point in the application
G_MODULE_EXPORT
std::shared_ptr<geryon::Application> createApplication() {
    std::shared_ptr<geryon::Application> app = std::make_shared<geryon::Application>("test");

    app->addFilter(std::make_shared<geryon::test::TestFilterGlobal>("*"));
    app->addFilter(std::make_shared<geryon::test::TestFilterNrm>("/filter/nrm/*"));

    return app;
}
