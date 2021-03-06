///
/// \file geryon_config.hpp
///
///  Created on: May 30, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef GERYON_GERYONCONFIG_HPP_
#define GERYON_GERYONCONFIG_HPP_

#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "server_application.hpp"

namespace geryon { namespace server {

struct GeryonAdminServerConfig {
    std::string service;
    bool enabled;
};

struct GeryonHttpServerConfig {
    std::string service;
    std::string bindAddress;
    unsigned int nAcceptors;
    unsigned int nExecutors;
    std::size_t maxRequestLength;
    bool trackConnections;
};

namespace detail {

struct GeryonConfigIssue {
    GeryonConfigIssue(bool _isFatal, const std::string & _message) : isFatal(_isFatal), message(_message) {}
    ~GeryonConfigIssue() {}
    bool isFatal;
    std::string message;
};

}

///
/// \brief The GeryonConfigurator class
/// We don't feel we need to create a builder here ... yet.
///
class GeryonConfigurator {
public:
    explicit GeryonConfigurator(const std::string & _homeBase,
                                unsigned int _serverId,
                                const std::string & _configFile,
                                const std::string & _modulesBase,
                                const std::string & _libsBase);
    ~GeryonConfigurator();

    bool configure();

    void unconfigure();

    inline GeryonAdminServerConfig & getAdminServerConfig() { return adminCfg; }

    inline std::vector<GeryonHttpServerConfig> & getHttpServers() { return httpServers; }

private:
    bool calculatePaths();

    bool configureLog();
    bool configureLibraries();
    bool configureMemoryStructs();
    bool configureResources();
    bool configureApplications();
    bool configureAdminServer();
    bool configureServers();

    void printIssues();
    std::map<std::string, std::string> propertiesFromNode(const boost::property_tree::ptree & node);
    std::shared_ptr<ServerApplication> configureApplication(const boost::property_tree::ptree & node);

    void unconfigureApplications();

    std::string homeBase;
    unsigned int serverId;
    const std::string configFile;
    const std::string modulesBase;
    const std::string libsBase;
    std::vector<detail::GeryonConfigIssue> issues;
    boost::property_tree::ptree configuration;
    boost::filesystem::path homeBasePath;
    boost::filesystem::path configFilePath;
    boost::filesystem::path modulesBasePath;
    boost::filesystem::path profileBasePath;
    boost::filesystem::path libsBasePath;

    GeryonAdminServerConfig adminCfg;
    std::vector<GeryonHttpServerConfig> httpServers;
};

} } /*namespace */

#endif
