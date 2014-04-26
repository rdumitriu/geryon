//
// MAIN ENTRY POINT (WIN & LIN)
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


//    std::thread * p_t = 0;
    try {

        // Block all signals for background thread.
        sigset_t new_mask;
        sigfillset(&new_mask);
        sigset_t old_mask;
        pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask);

        // Run server in a subsequent background thread.
//        geryon::server::TCPServer server("httpserver", "127.0.0.1", "7000", 2);
//        geryon::server::TCPServer * pServer = &server;

//        p_t = new std::thread([pServer]() {
//            pServer->operator()();
//        });

        // Restore previous signals.
        pthread_sigmask(SIG_SETMASK, &old_mask, 0);

        // Wait for signal indicating time to shut down.
        sigset_t wait_mask;
        sigemptyset(&wait_mask);
        sigaddset(&wait_mask, SIGHUP);
        sigaddset(&wait_mask, SIGINT);
        sigaddset(&wait_mask, SIGQUIT);
        sigaddset(&wait_mask, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &wait_mask, 0);
        int sig = 0;
        sigwait(&wait_mask, &sig); //wait forever

        // Stop it.
//        server.stop();
//        p_t->join();
//        delete p_t;
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
//        delete p_t;
    } catch( ... ) {
        std::cerr << "exception: general, unspecified " << std::endl;
//        delete p_t;
    }

    return 0;
}
