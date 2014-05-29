
#include "server_application_utils.hpp"
#include "string_utils.hpp"

namespace geryon { namespace server { namespace detail {

std::string concatPaths(const std::string & first, const std::string & second) {
   using namespace geryon::util;
   if(endsWith(first, "/") && startsWith(second, "/")) {
       if(first == "/") {
           return second;
       }
       //we need to adjust
       std::string nosl = first.substr(0, first.size()-1);
       return nosl + second;
   } else if(endsWith(first, "/") || startsWith(second, "/")) {
       //no matter who has the slash, this works
       return first + second;
   }
   return first + "/" + second;
}

std::string calculatePathFromModule(const ApplicationModule & appm, const std::string &last) {
    ApplicationModule * pApp = const_cast<ApplicationModule *>(&appm);
    std::string ret("");
    while(pApp) {
        ret = concatPaths(pApp->getConfig().getMountPath(), ret);
        pApp = pApp->getParent();
    }
    return concatPaths(ret, last);
}

std::string createRegexFromPath(const std::string & path) {
    std::string ret;
    //escape the . and instead of * add '.*'
    char * pc = const_cast<char *>(path.c_str());
    while(*pc) {
        switch(*pc) {
            case '.':
                ret.push_back('\\');
                ret.push_back('.');
                break;
            case '*':
                ret.push_back('.');
                ret.push_back('*');
                break;
            default:
                ret.push_back(*pc);
                break;
        }
        pc++;
    }
    return ret;
}

} } }
