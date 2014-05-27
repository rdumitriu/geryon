//
// MAIN ENTRY POINT (LIN)
//

//linux only
#include <pthread.h>
#include <signal.h>

//general
#include <iostream>
#include <string>
#include <thread>

#include <boost/program_options.hpp>

#include "server_build.hpp"
#include "tcp_server.hpp"
#include "server_global_structs.hpp"
#include "tcp_sta_server.hpp"
#include "tcp_mta_server.hpp"
#include "gadm_protocol.hpp"
#include "http_protocol.hpp"
#include "http_executor.hpp"
#include "http_st_executor.hpp"
#include "http_mt_executor.hpp"


#include "log.hpp"

int main(int argc, char* argv[]) {
    boost::program_options::options_description desc("Command-line options");
    desc.add_options()
        ("help", "Produces this help message.")
        ("home,h", "Specifies the home directory")
        ("config,c", boost::program_options::value<std::string>()->implicit_value(std::string("")),
                    "Specifies the configuration file. Optional")
        ("modulesDir,m", boost::program_options::value<std::string>()->implicit_value(std::string("")),
                    "Specifies the modules directory. Optional")
    ;

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help") || vm.empty()) {
        std::cout << desc << std::endl;
        return 1;
    }

    std::cout << "Booting " << GERYON_VERSION_FULL_STRING << std::endl;
    if(vm.count("home")) {
        std::cout << "Home directory set to:" << vm["home"].as<std::string>() << std::endl;
    }

    geryon::util::Log::configureBasic(geryon::util::Log::DEBUG);

    std::shared_ptr<geryon::server::GMemoryPool> pool(new geryon::server::GUniformMemoryPool(1024));
    geryon::server::ServerGlobalStucts::setMemoryPool(pool);


    try {
        //Protocol is 'gadm'
        geryon::server::GAdmProtocol adminProtocol;
        //Start the admin server on port 7001
        geryon::server::SingleThreadAcceptorTCPServer adminserver("adminserver", "127.0.0.1", "7001", adminProtocol);
        geryon::server::SingleThreadAcceptorTCPServer * pAdminServer = &adminserver;

        std::shared_ptr<std::thread> p_admin_thr (new std::thread([pAdminServer]() {
            pAdminServer->run();
        }));

        //HTTP server: Run server in a subsequent background thread, as normal.
        //MT server (there are N acceptor threads)
        geryon::server::HttpMultiThreadExecutor executor(2);
        geryon::server::HttpProtocol httpProtocol(&executor);
        geryon::server::MultiThreadedAcceptorTCPServer httpserver("httpserver", "127.0.0.1", "8001", httpProtocol, 6);
        geryon::server::MultiThreadedAcceptorTCPServer * pHttpServer = &httpserver;

        std::shared_ptr<std::thread> p_http_thr (new std::thread([pHttpServer]() {
            pHttpServer->run();
        }));

        p_http_thr->join();
        p_admin_thr->join();
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    } catch( ... ) {
        std::cerr << "exception: general, unspecified " << std::endl;
    }

    return 0;
}
