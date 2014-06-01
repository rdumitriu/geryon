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
#include "geryon_config.hpp"

class GServer {
public:
    GServer() {}
    virtual ~GServer() {}
    std::shared_ptr<std::thread> getThread() { return thr; }

    void await() {
        thr->join();
    }

protected:
    std::shared_ptr<std::thread> thr;
};

class GAdminServer : public GServer {
public:
    GAdminServer(const std::string & service)
            : GServer(),
              adminProtocol(),
              adminserver("adminserver", "127.0.0.1", service, adminProtocol) {
        geryon::server::SingleThreadAcceptorTCPServer * pAdminServer = &adminserver;

        thr = std::shared_ptr<std::thread>(new std::thread([pAdminServer]() {
            pAdminServer->run();
        }));
    }
    virtual ~GAdminServer() {}
private:
    geryon::server::GAdmProtocol adminProtocol;
    geryon::server::SingleThreadAcceptorTCPServer adminserver;
};

class GHttpServer : public GServer {
public:
    GHttpServer(unsigned int nExecThreads, std::size_t maxRequestLen)
            : GServer(),
              executor(nExecThreads),
              httpProtocol(executor, maxRequestLen) {
    }
    virtual ~GHttpServer() {}

protected:
    geryon::server::HttpMultiThreadExecutor executor;
    geryon::server::HttpProtocol httpProtocol;
};



class GMHttpServer : public GHttpServer {
public:
    GMHttpServer(const std::string & bindAddress, const std::string & service,
                 unsigned int nExecThreads, std::size_t maxRequestLen,
                 unsigned int nAcceptorThreads)
            : GHttpServer(nExecThreads, maxRequestLen),
              httpserver("httpserver", bindAddress, service, httpProtocol, nAcceptorThreads) {
        geryon::server::MultiThreadedAcceptorTCPServer * pHttpServer = &httpserver;

        thr = std::shared_ptr<std::thread>(new std::thread([pHttpServer]() {
            pHttpServer->run();
        }));
    }
    virtual ~GMHttpServer() {}

private:
    geryon::server::MultiThreadedAcceptorTCPServer httpserver;
};

class GSHttpServer : public GHttpServer {
public:
    GSHttpServer(const std::string & bindAddress, const std::string & service,
                 unsigned int nExecThreads, std::size_t maxRequestLen)
            : GHttpServer(nExecThreads, maxRequestLen),
              httpserver("httpserver", bindAddress, service, httpProtocol) {
        geryon::server::SingleThreadAcceptorTCPServer * pHttpServer = &httpserver;

        thr = std::shared_ptr<std::thread>(new std::thread([pHttpServer]() {
            pHttpServer->run();
        }));
    }
    virtual ~GSHttpServer() {}

private:
    geryon::server::SingleThreadAcceptorTCPServer httpserver;
};


int main(int argc, char* argv[]) {
    unsigned int serverId = 0;
    std::string home;
    std::string config;
    std::string modules;

    boost::program_options::options_description desc("Command-line options");
    desc.add_options()
        ("help", "Produces this help message.")
        ("home,h", boost::program_options::value<std::string>()->required(),
                   "Specifies the home directory")
        ("config,c", boost::program_options::value<std::string>()->implicit_value(std::string("")),
                    "Configuration file. [<GERYON_HOME>/profiles/server<SID>/server.conf]")
        ("modules,m", boost::program_options::value<std::string>()->implicit_value(std::string("")),
                    "Specifies the modules directory [<GERYON_HOME>/modules/]")
        ("serverId,i", boost::program_options::value<unsigned int>()->implicit_value(0),
                    "Specifies the server id (SID).")
    ;

    try {
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help") || vm.empty()) {
            std::cout << desc << std::endl;
            return 1;
        }


        if(vm.count("home")) {
            home = vm["home"].as<std::string>();
            std::cout << "Home directory set to:" << vm["home"].as<std::string>() << std::endl;
        } else {
            std::cout << desc << std::endl;
            return 1;
        }
        if(vm.count("config")) {
            config = vm["config"].as<std::string>();
        }
        if(vm.count("modules")) {
            modules = vm["modules"].as<std::string>();
        }
        if(vm.count("serverId")) {
            serverId = vm["serverId"].as<unsigned int>();
        }
    } catch( ... ) {
        std::cout << desc << std::endl;
        return 1;
    }

    std::cout << "Booting " << GERYON_VERSION_FULL_STRING << " Server Id: " << serverId << std::endl;

    geryon::server::GeryonConfigurator configurator(home, serverId, config, modules);
    if(!configurator.configure()) {
        return 1;
    }

    std::vector<std::shared_ptr<GServer>> servers;

    try {
        if(configurator.getAdminServerConfig().enabled) {
            servers.push_back(std::make_shared<GAdminServer>(configurator.getAdminServerConfig().service));
        }

        for(auto cfg : configurator.getHttpServers()) {
            if(cfg.nAcceptors <= 1) {
                servers.push_back(std::make_shared<GSHttpServer>(cfg.bindAddress,
                                                                 cfg.service,
                                                                 cfg.nExecutors,
                                                                 cfg.maxRequestLength));
            } else {
                servers.push_back(std::make_shared<GMHttpServer>(cfg.bindAddress,
                                                                 cfg.service,
                                                                 cfg.nExecutors,
                                                                 cfg.maxRequestLength,
                                                                 cfg.nAcceptors));
            }
        }

        for(std::vector<std::shared_ptr<GServer>>::iterator i = servers.begin(); i != servers.end(); ++i) {
            (*i)->await();
        }
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << std::endl;
    } catch( ... ) {
        std::cerr << "exception: general, unspecified " << std::endl;
    }

    configurator.unconfigure();

    return 0;
}
