
#include "os_utils.hpp"
#include "geryon_configapp.hpp"

#include "log.hpp"

namespace geryon { namespace server {

class GeryonConfigurationInjector : public geryon::configuration::ApplicationConfigInjector {
public:
    GeryonConfigurationInjector(std::string & _path, std::map<std::string, std::string> & _props) : path(_path), props(_props) {
    }

    virtual std::string getMountPath() { return path; }

    virtual std::map<std::string, std::string> getProperties() { return props; }

private:
    std::string & path;
    std::map<std::string, std::string> & props;
};

typedef std::shared_ptr<geryon::Application> (*pFunctCreateApp)();

typedef std::shared_ptr<geryon::ApplicationModule> (*pFunctCreatePlugin)();


GeryonAppBaseConfigurator::GeryonAppBaseConfigurator(const boost::filesystem::path & modulesBasePath,
                                                     const std::string & module) : moduleDLL(0) {
#if defined(G_HAS_WIN)
    boost::filesystem::path soFile = modulesBasePath / (module + ".dll");
#else
    boost::filesystem::path soFile = modulesBasePath / (module + ".so");
#endif
    if(boost::filesystem::exists(soFile) && boost::filesystem::is_regular_file(soFile)) {
        moduleDLL = openDynamicLibrary(soFile.c_str());
    } else {
        LOG(geryon::util::Log::ERROR) << "Could not load dynamic library :" << soFile.c_str();
    }
}

bool GeryonAppBaseConfigurator::isConfigurationValid() {
    return (moduleDLL != 0);
}

GeryonAppBaseConfigurator::~GeryonAppBaseConfigurator() {
    if(moduleDLL) {
        closeDynamicLibrary(moduleDLL);
    }
}


GeryonApplicationConfigurator::GeryonApplicationConfigurator(const boost::filesystem::path & modulesBasePath,
                                                             GeryonApplicationConfig & _cfg)
        : GeryonAppBaseConfigurator(modulesBasePath, _cfg.module), cfg(_cfg) {
    void * createAppFunc;
    #if defined(G_HAS_WIN)
        createAppFunc = reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HINSTANCE>(moduleDLL), "createApplication"));
    #else
        createAppFunc = dlsym(moduleDLL, "createApplication");
    #endif
    if(!createAppFunc) {
        LOG(geryon::util::Log::WARNING) << "Cannot find application's entry point in library [" << cfg.module << "]";
    } else {
        pFunctCreateApp pfun = reinterpret_cast<pFunctCreateApp>(reinterpret_cast<size_t>(createAppFunc));
        pApplication = pfun();

        if(!pApplication.get()) {
            LOG(geryon::util::Log::WARNING) << "NULL application in library [" << cfg.module << "] ?!?";
        }
    }
}

GeryonApplicationConfigurator::~GeryonApplicationConfigurator() {
}

bool GeryonApplicationConfigurator::isConfigurationValid() {
    return GeryonAppBaseConfigurator::isConfigurationValid() && (0 != pApplication.get());
}

std::shared_ptr<ServerApplication> GeryonApplicationConfigurator::application() {
    GeryonConfigurationInjector injector(cfg.path, cfg.props);
    pApplication->getConfig().setup(injector);
    return std::make_shared<ServerApplication>(pApplication,
                                               cfg.sessPartitions,
                                               cfg.sessTimeOut,
                                               cfg.sessCleanupInterval);
}

void GeryonApplicationConfigurator::addModule(std::shared_ptr<ApplicationModule> module_ptr) {
    pApplication->addModule(module_ptr);
}




GeryonApplicationModuleConfigurator::GeryonApplicationModuleConfigurator(const boost::filesystem::path & modulesBasePath,
                                                                         GeryonApplicationModuleConfig & _cfg)
        : GeryonAppBaseConfigurator(modulesBasePath, _cfg.module), cfg(_cfg) {
    void * createPluginFunc;
    #if defined(G_HAS_WIN)
        createPluginFunc = reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HINSTANCE>(moduleDLL), "createPlugin"));
    #else
        createPluginFunc = dlsym(moduleDLL, "createPlugin");
    #endif
    if(!createPluginFunc) {
        LOG(geryon::util::Log::WARNING) << "Cannot find plugin's entry point in library [" << cfg.module << "]";
    } else {
        pFunctCreatePlugin pfun = reinterpret_cast<pFunctCreatePlugin>(reinterpret_cast<size_t>(createPluginFunc));
        pPlugin = pfun();

        if(!pPlugin.get()) {
            LOG(geryon::util::Log::WARNING) << "NULL plugin in library [" << cfg.module << "] ?!?";
        }
    }
}

GeryonApplicationModuleConfigurator::~GeryonApplicationModuleConfigurator() {
}

bool GeryonApplicationModuleConfigurator::isConfigurationValid() {
    return GeryonAppBaseConfigurator::isConfigurationValid() && (0 != pPlugin.get());
}

std::shared_ptr<ApplicationModule> GeryonApplicationModuleConfigurator::module() {
    GeryonConfigurationInjector injector(cfg.path, cfg.props);
    pPlugin->getConfig().setup(injector);
    return pPlugin;
}


} }
