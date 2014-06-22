///
/// \file geryon_config.hpp
///
///  Created on: May 31, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef GERYON_GERYONCONFIGAPP_HPP_
#define GERYON_GERYONCONFIGAPP_HPP_

#include <map>
#include <string>

#include <boost/filesystem.hpp>
#include "server_application.hpp"


namespace geryon { namespace server {

struct GeryonApplicationConfig {
    std::string path;
    std::string module;
    unsigned int sessTimeOut;
    unsigned int sessPartitions;
    unsigned int sessCleanupInterval;
    std::map<std::string, std::string> props;
};

struct GeryonApplicationModuleConfig {
    std::string path;
    std::string module;
    std::map<std::string, std::string> props;
};

class GeryonAppBaseConfigurator {
protected:
    GeryonAppBaseConfigurator(const boost::filesystem::path & modulesBasePath, const std::string & module);
    virtual ~GeryonAppBaseConfigurator();
public:
    bool isConfigurationValid();
    void * getModuleDLL();
protected:
    void * moduleDLL;
};

class GeryonApplicationConfigurator : public GeryonAppBaseConfigurator {
public:
    GeryonApplicationConfigurator(const boost::filesystem::path & modulesBasePath,
                                  GeryonApplicationConfig & cfg);
    ~GeryonApplicationConfigurator();
    bool isConfigurationValid();
    std::shared_ptr<ServerApplication> application();

    void addModule(std::shared_ptr<ApplicationModule> module_ptr);
private:
    std::shared_ptr<Application> pApplication;
    GeryonApplicationConfig & cfg;
};

class GeryonApplicationModuleConfigurator : public GeryonAppBaseConfigurator {
public:
    GeryonApplicationModuleConfigurator(const boost::filesystem::path & modulesBasePath,
                                        GeryonApplicationModuleConfig & cfg);
    ~GeryonApplicationModuleConfigurator();
    bool isConfigurationValid();
    std::shared_ptr<ApplicationModule> module();

private:
    std::shared_ptr<ApplicationModule> pPlugin;
    GeryonApplicationModuleConfig & cfg;
};

} } /*namespace */

#endif
