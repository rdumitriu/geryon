
#include "server_application_utils.hpp"
#include "string_utils.hpp"

namespace geryon { namespace server { namespace detail {

std::string concatPaths(const std::string & first, const std::string & second) {
    using namespace geryon::util;
    if("" == second) {
        return first;
    }
    if("" == first) {
        return second;
    }
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

unsigned int countStars(const std::string & path) {
    unsigned int ret = 0;
    char * pc = const_cast<char *>(path.c_str());
    while(*pc) {
        if(*pc == '*') {
            ++ret;
        }
        pc++;
    }
    return ret;
}

MatchingEntry createMatchingEntry(const std::string & path) {
    MatchingEntry ret;
    ret.mappedPath = path;
    ret.noStars = countStars(path);
    switch(ret.noStars) {
        case 0:
            //direct match
            break;
        case 1:
            //split the string around the star
            {
                auto ndx = path.find("*");
                ret.beginOfPath = path.substr(0, ndx);
                if(ndx < path.length() - 1) {
                    ret.endOfPath = path.substr(ndx+1);
                }
            }
            break;
        default:
            //slow, regex
            ret.regex = std::regex(detail::createRegexFromPath(path));
            break;
    }
    return ret;
}

bool isMatchingEntry(const std::string & path, const MatchingEntry & mentry) {
    switch(mentry.noStars) {
        case 0:
            return path == mentry.mappedPath;
        case 1:
            {
                if(!mentry.beginOfPath.empty() && !mentry.endOfPath.empty()) {
                    return geryon::util::startsWith(path, mentry.beginOfPath) &&
                           geryon::util::endsWith(path, mentry.endOfPath);
                } else if(!mentry.beginOfPath.empty()) {
                    return geryon::util::startsWith(path, mentry.beginOfPath);
                } else if(!mentry.endOfPath.empty()) {
                    return geryon::util::endsWith(path, mentry.endOfPath);
                }
                return true; //both are empty, '*' matches everything!
            }
            break;
        default:
            return std::regex_match(path, mentry.regex);
    }
    return false;
}

} } }
