
#include <cstdlib>

#include "server_build.hpp"
#include "geryon_config.hpp"
#include "geryon_configapp.hpp"

#ifdef G_HAS_PQXX
#include "postgres_support.hpp"
#endif

#include "geryon_configsql_postgres.hpp"

#include "server_global_structs.hpp"

#include "os_utils.hpp"

#include "log.hpp"

namespace geryon { namespace server {

//::TODO:: lots of checks should be in place

GeryonConfigurator::GeryonConfigurator(const std::string & _homeBase,
                                       unsigned int _serverId,
                                       const std::string & _configFile,
                                       const std::string & _modulesBase,
                                       const std::string & _libsBase)
                    : homeBase(_homeBase), serverId(_serverId), configFile(_configFile),
                      modulesBase(_modulesBase), libsBase(_libsBase) {
}

GeryonConfigurator::~GeryonConfigurator() {
}

bool GeryonConfigurator::calculatePaths() {
    homeBasePath = boost::filesystem::path(homeBase);
    homeBasePath = boost::filesystem::absolute(homeBasePath);
    if(!boost::filesystem::exists(homeBasePath) || !boost::filesystem::is_directory(homeBasePath)) {
        std::cerr << "Home directory " << homeBasePath.c_str() << " does not exist!" << std::endl;
        return false;
    }
    if(configFile == "") {
        std::ostringstream srvprof;
        srvprof << "server" << serverId;
        configFilePath = homeBasePath / "profiles" / srvprof.str() / "etc" / "server.conf";
        profileBasePath = homeBasePath / "profiles" / srvprof.str();
    } else {
        configFilePath = boost::filesystem::path(configFile);
        profileBasePath = configFilePath.parent_path();
    }
    if(!boost::filesystem::exists(profileBasePath) || !boost::filesystem::is_directory(profileBasePath)) {
        std::cerr << "Profile directory " << profileBasePath.c_str() << " does not exist!" << std::endl;
        return false;
    }
    if(!boost::filesystem::exists(configFilePath) || !boost::filesystem::is_regular_file(configFilePath)) {
        std::cerr << "Configuration file " << configFilePath.c_str() << " does not exist!" << std::endl;
        return false;
    }
    if(modulesBase == "") {
        modulesBasePath = homeBasePath / "modules";
    } else {
        modulesBasePath = boost::filesystem::path(modulesBase);
    }
    if(!boost::filesystem::exists(modulesBasePath) || !boost::filesystem::is_directory(modulesBasePath)) {
        std::cerr << "Modules directory " << modulesBasePath.c_str() << " does not exist!" << std::endl;
        return false;
    }
    if(libsBase == "") {
        libsBasePath = homeBasePath / "lib";
    } else {
        libsBasePath = boost::filesystem::path(libsBase);
    }
    if(!boost::filesystem::exists(libsBasePath) || !boost::filesystem::is_directory(libsBasePath)) {
        std::cerr << "Libraries directory " << libsBasePath.c_str() << " does not exist!" << std::endl;
        return false;
    }
    return true;
}

bool GeryonConfigurator::configure() {
    if(!calculatePaths()) {
        return false;
    }
    try {
        boost::property_tree::read_xml(configFilePath.c_str(), configuration);
    }
    catch (std::exception &e) {
        std::cerr << "Error reading configuration: " << e.what() << std::endl;
        return false;
    }

    bool ret = configureLog() &&
               configureLibraries() &&
               configureMemoryStructs() &&
               configureResources() &&
               configureApplications() &&
               configureAdminServer() &&
               configureServers();
    if(!issues.empty()) {
        printIssues();
    }
    return ret;
}


bool GeryonConfigurator::configureLog() {
    std::string cfgLevel = configuration.get("geryon.log.level", "INFO");
    std::string file = configuration.get("geryon.log.file", "geryon.log");

    boost::filesystem::path logBase = profileBasePath / "log";
    if(!boost::filesystem::exists(logBase)) {
        logBase = profileBasePath;
    }
    boost::filesystem::path logFilePath = logBase / file;

    geryon::util::Log::configureFile(geryon::util::Log::fromString(cfgLevel), logFilePath.c_str());

    LOG(geryon::util::Log::INFO) << "======================================";
    LOG(geryon::util::Log::INFO) << "Starting " << GERYON_VERSION_FULL_STRING << " - server ID :" << serverId << " LL:" << cfgLevel;
    return true;
}

bool GeryonConfigurator::configureLibraries() {
    boost::property_tree::ptree rootNodes = configuration.get_child("geryon");
    for (const auto& kv : rootNodes) {
        if(kv.first == "library") {
            boost::property_tree::ptree node = kv.second;
            std::string libname = node.get_value_optional<std::string>().get();
        #if defined(G_HAS_WIN)
            boost::filesystem::path soFile = libsBasePath / (libname + ".dll");
        #else
            boost::filesystem::path soFile = libsBasePath / (libname + ".so");
        #endif
            if(boost::filesystem::exists(soFile) && boost::filesystem::is_regular_file(soFile)) {
                void * mod_handler = openDynamicLibrary(soFile.c_str());
                if(mod_handler) {
                    geryon::server::ServerGlobalStructs::addModuleDLL(mod_handler);
                }
            } else {
                LOG(geryon::util::Log::ERROR) << "Could not load dynamic library:" << soFile.c_str();
            }
        }
    }
    return true;
}

bool GeryonConfigurator::configureMemoryStructs() {
    ServerGlobalStructs::setServerId(serverId);
    std::string sendServerToken = configuration.get("geryon.brag", "y");
    if(sendServerToken == "y") {
        ServerGlobalStructs::setServerToken(GERYON_VERSION_STRING);
    }
    std::size_t buffSz = configuration.get("geryon.buffers.pagesize", 4096);
    std::size_t buffInitial = configuration.get("geryon.buffers.initial", 0);
    std::size_t buffMaximal = configuration.get("geryon.buffers.max", 0);

    std::shared_ptr<geryon::server::GMemoryPool> pool(new geryon::server::GUniformMemoryPool(buffSz, buffInitial, buffMaximal));
    geryon::server::ServerGlobalStructs::setMemoryPool(pool);
    return true;
}

bool GeryonConfigurator::configureResources() {
    boost::property_tree::ptree rootNodes = configuration.get_child("geryon");
    for (const auto& kv : rootNodes) {
        if(kv.first == "sql-pool") {
            boost::property_tree::ptree sqlNode = kv.second;
            std::string name = sqlNode.get("name", "");
            if(name == "") {
                issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                                     "No name provided for sql pool; skipped.")));
            }
            std::string type = sqlNode.get("type", "");
            unsigned int minSize = sqlNode.get("min-size", 1);
            unsigned int maxSize =  sqlNode.get("max-size", 10);
            unsigned int connectionTTL = sqlNode.get("connection-ttl", 3600);
            unsigned int maintenanceInterval = sqlNode.get("maintenance-interval", 300);
            bool testOnBorrow = sqlNode.get("test-on-borrow", false);
            bool testOnReturn = sqlNode.get("test-on-return", false);

            if(type == "postgres" || type == "psql") {
#ifdef G_HAS_PQXX
                std::string dbhost = sqlNode.get("host", "");
                std::string dbport = sqlNode.get("port", "");
                std::string dbname = sqlNode.get("dbname", "");
                std::string user = sqlNode.get("user", "");
                std::string password = sqlNode.get("password", "");
                std::string dboptions = sqlNode.get("options", "");

                geryon::server::GeryonPostgresConfigurator postgres_cfg(name);
                postgres_cfg.setSQLParams(dbhost, dbport, dbname, user, password, dboptions);
                postgres_cfg.setPoolParams(minSize, maxSize, connectionTTL, maintenanceInterval, testOnBorrow, testOnReturn);
                postgres_cfg.configure();
#else
                issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                                     "Server does not have support for " + type +
                                                                     ". Recompile it. Skipped SQL pool :" + name)));
#endif
            } else {
                issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                                     "No known type specified (" + type +
                                                                     ") for sql pool :" + name)));
            }
        }
    }
    return true;
}

bool GeryonConfigurator::configureApplications() {
    boost::property_tree::ptree rootNodes = configuration.get_child("geryon");
    for (const auto& kv : rootNodes) {
        if(kv.first == "application") {
            try {
                std::shared_ptr<ServerApplication> appptr = configureApplication(kv.second);
                if(appptr.get()) {
                    appptr->start();
                    geryon::server::ServerGlobalStructs::defineApplication(appptr);
                }
            } catch(geryon::ApplicationException & e) {
                LOG(geryon::util::Log::ERROR) << "Could not start application, error was:" << e.what();
            } catch( ... ) {
                LOG(geryon::util::Log::ERROR) << "Could not start an application; please connect to the console to see what failed.";
            }
        }
    }
    return true;
}

std::shared_ptr<ServerApplication> GeryonConfigurator::configureApplication(const boost::property_tree::ptree & node) {
    std::shared_ptr<ServerApplication> app;
    GeryonApplicationConfig cfg;
    cfg.path = node.get("path", "");
    cfg.module = node.get("module", "");
    cfg.sessTimeOut = node.get("session.time-out", 1800);
    cfg.sessPartitions = node.get("session.partitions", 25);
    cfg.sessCleanupInterval = node.get("session.cleanup-interval", 300);
    cfg.props = propertiesFromNode(node);
    if(cfg.path == "") {
        issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                             "Application >>" + cfg.module +
                                                             "<< has no path! Skipped.")));
        return app;
    }
    if(cfg.module == "") {
        issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                             "Application mounted on >>" + cfg.module +
                                                             "<< has no module! Skipped.")));
        return app;
    }
    GeryonApplicationConfigurator app_configurer(modulesBasePath, cfg);
    if(app_configurer.isConfigurationValid()) {
        app = app_configurer.application();
        geryon::server::ServerGlobalStructs::addModuleDLL(app_configurer.getModuleDLL());
        LOG(geryon::util::Log::INFO) << "Loaded application " << cfg.module << " to be mounted on path :" << cfg.path;
        //now, the modules
        for (const auto& kv : node) {
            if(kv.first == "plugin") {
                GeryonApplicationModuleConfig plgcfg;
                plgcfg.path = kv.second.get("path", "");
                plgcfg.module = kv.second.get("module", "");
                plgcfg.props = propertiesFromNode(kv.second);
                if(plgcfg.module == "") {
                    issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                                         "Application mounted on >>" + cfg.module +
                                                                         "<< has no an non-configurable plugin! Skipped.")));
                } else {
                    GeryonApplicationModuleConfigurator plg_configurer(modulesBasePath, plgcfg);
                    if(plg_configurer.isConfigurationValid()) {
                        app_configurer.addModule(plg_configurer.module());
                        geryon::server::ServerGlobalStructs::addModuleDLL(plg_configurer.getModuleDLL());
                    } else {
                        issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                                             "Module plugin " + plgcfg.module + ", host application >>" + cfg.module +
                                                                             "<< cannot be instantiated! Skipped.")));
                    }
                }
            }
        }
    } else {
        issues.push_back(std::move(detail::GeryonConfigIssue(false,
                                                             "Application mounted on >>" + cfg.module +
                                                             "<< cannot be instantiated! Skipped.")));
    }
    return app;
}

std::map<std::string, std::string> GeryonConfigurator::propertiesFromNode(const boost::property_tree::ptree & node) {
    std::map<std::string, std::string> ret;
    for (const auto& kv : node) {
        if(kv.first == "property") {
            std::string key = kv.second.get("name", "");
            std::string value = kv.second.get("value", "");
            if(key != "") {
                ret.insert(std::make_pair(key, value));
            }
        }
    }
    return ret;
}

bool GeryonConfigurator::configureAdminServer() {
    std::string adminService = configuration.get("geryon.admin.service", "7001");
    std::string adminServiceEnabled = configuration.get("geryon.admin.enabled", "y");

    adminCfg.enabled = adminServiceEnabled == "y" && adminService != "";
    adminCfg.service = adminService;
    return true;
}

bool GeryonConfigurator::configureServers() {
    boost::property_tree::ptree rootNodes = configuration.get_child("geryon");
    for (const auto& kv : rootNodes) {
        //kw.first //node
        //kw.second //tree
        if(kv.first == "server") {
            GeryonHttpServerConfig cfg;
            boost::property_tree::ptree serverNode = kv.second;
            cfg.bindAddress = serverNode.get("bind", "*");
            cfg.service = serverNode.get("port", "8081");
            cfg.nExecutors = serverNode.get("threads", 25);
            cfg.nAcceptors = serverNode.get("acceptors", 2);
            cfg.maxRequestLength = serverNode.get("max-req-len", 10485760);
            cfg.trackConnections = serverNode.get("track-connections", false);

            httpServers.push_back(std::move(cfg));
        }
    }
    //::TODO:: check duplicate ports, empty values, etc
    if(httpServers.size() == 0) {
        issues.push_back(std::move(detail::GeryonConfigIssue(true, "There is no http server configured ?")));
    }
    return true;
}

void GeryonConfigurator::printIssues() {
    std::cerr << "There are issues in the configuration !" << std::endl;
    for(auto & i : issues) {
        std::cerr << i.message << std::endl;
    }
}


void GeryonConfigurator::unconfigure() {
    //apps
    unconfigureApplications();

    ServerGlobalStructs::clear();
}

void GeryonConfigurator::unconfigureApplications() {
    for(const auto & appptr : geryon::server::ServerGlobalStructs::getApplications()) {
        try {
            appptr->stop();
        } catch( ... ) {
            LOG(geryon::util::Log::ERROR) << "Error stopping application mapped on :" << appptr->getPath();
        } //ignored.
    }
}

} }
