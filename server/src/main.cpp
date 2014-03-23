//
// MAIN ENTRY POINT (WIN & LIN)
//
#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include "server_build.hpp"

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

    return 0;
}
