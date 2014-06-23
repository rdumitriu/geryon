/**
 * \file string_utils.cpp
 *
 * Created on: Feb 25, 2014
 * Author: rdumitriu at gmail.com
 */

#include "string_utils.hpp"


namespace geryon { namespace util {

/* ==================== HTTP UTILITIES ====================================== */

namespace http {

const char http_special_chars[] = {'(', ')', '<', '>', '@', ',', ';', ':', '\\', '"', '/', '[', ']', '?', '=', '{', '}', ' ', '\t', 0};

bool isHTTPSpecial(char c) {
    char * p = const_cast<char *>(http_special_chars);
    while(*p) {
        if(c == *p) {
            return true;
        }
        p++;
    }
    return false;
}

} /* namespace http */


bool decodeURL(const std::string& in, std::string& out) {
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return true;
}

void encodeURL(const std::string& in, std::string& out) {
    out.clear();
    out.reserve(in.size());

    for (std::size_t pos = 0; pos < in.size(); ++pos) {
        char c = in[pos];
        switch(c) {
            default:
                if (c > 32 && c < 127) {
                    // character does not need to be escaped
                    out.push_back(c);
                    break;
                }
            case ' ': case '$': case '&': case '+': case ',': case '/': case ':':
            case ';': case '=': case '?': case '@': case '"': case '<':
            case '>': case '#': case '%': case '{': case '}': case '|':
            case '\\': case '^': case '~': case '[': case ']': case '`':
                {
                    std::ostringstream os;
                    os << "%" << std::hex << c;
                    out += os.str();
                    break;
                }
            }
    }
}

} } /*namespace*/
