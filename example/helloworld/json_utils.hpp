///
/// \file json_utils.hpp
///
/// \brief Defines the utility routines used internally by JSON
///
/// Created on: Jun 23, 2014
/// Author: rdumitriu at gmail.com
///
#ifndef GERYON_JSON_UTILS_HPP_
#define GERYON_JSON_UTILS_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <memory>

namespace ll1gen { namespace json {

namespace detail {

void encodeJsonString(const std::string & orig, std::string & final) {
    char * c = const_cast<char *>(orig.c_str());
    while(*c) {
        switch(*c) {
            case '\"':
                final.append("\\\"");
                break;
            case '\\':
                final.append("\\\\");
                break;
            case '/':
                final.append("\\/");
                break;
            case '\b':
                final.append("\\b");
                break;
            case '\n':
                final.append("\\n");
                break;
            case '\f':
                final.append("\\f");
                break;
            case '\r':
                final.append("\\r");
                break;
            case '\t':
                final.append("\\t");
                break;
            default:
                if(0x00 <= *c && *c <= 0x1F) {
                    char buf[8];
                    sprintf(buf,"\\u%04X",*c);
                    final.append(buf);
                } else {
                    final.push_back(*c);
                }
                break;
        }
        ++c;
    }
}

std::string encodeJsonString(const std::string & orig) {
    std::string final;
    encodeJsonString(orig, final);
    return final;
}

char lookUpAndEat(std::istream& stream, const std::string & ends) {
    bool found = false;
    char c = 0;
    do {
        c = stream.get();
        if(stream.bad()) {
            throw std::runtime_error("Encountered premature EOF");
        }
//        std::cout << "Char >" << c << "<" << std::endl;
        if(!isblank(c) && c != '\n' && c != '\r') {
            found = (ends.find(c) != std::string::npos);
            if(!found) {
                std::ostringstream msg;
                msg << "Parse error, found char >>" << c << "<< but was expecting " << ends;
                throw std::runtime_error(msg.str());
            }
        }
    } while( !found );
    return c;
}

void lookUpTokenRemainder(std::istream& stream, char start, const std::string & token) {
    if(token.at(0) != start) {
        std::ostringstream msg;
        msg << "Parse error, found char >>" << start << "<< but was expecting " << token;
        throw std::runtime_error(msg.str());
    }
    unsigned int i = 1;
    while(i < token.size()) {
        char c = stream.get();
        if(stream.bad()) {
            throw std::runtime_error("Encountered premature EOF");
        }
        if(c != token.at(i)) {
            std::ostringstream msg;
            msg << "Parse error, found char >>" << c << "<< but was expecting " << token
                << ", char:" << token.at(i) << " position :" << i;
            throw std::runtime_error(msg.str());
        }
        ++i;
    }
}

std::string lookUp(std::istream& stream, const std::string & ends) {
    bool found = false;
    int state = 0;
    std::string ret;
    char c = 0;
    char unicodeBuff[5];
    int unicodeLen = 0;

    do {
        c = stream.get();
        if(stream.bad()) {
            throw std::runtime_error("Encountered premature EOF");
        }
//        std::cout << "Char >" << c << "< State :" << state << std::endl;

        switch(state) {
            case 1:
                switch(c) {
                    case 'b':
                        state = 0;
                        ret.push_back('\b');
                        break;
                    case 'n':
                        state = 0;
                        ret.push_back('\n');
                        break;
                    case 'f':
                        state = 0;
                        ret.push_back('\f');
                        break;
                    case 'r':
                        state = 0;
                        ret.push_back('\r');
                        break;
                    case 't':
                        state = 0;
                        ret.push_back('\t');
                        break;
                    case 'u':
                        unicodeLen = 0;
                        state = 2;
                        break;
                    default:
                        state = 0;
                        ret.push_back(c);
                        break;
                }
                break;
            case 2: //unicode: next 4 chars are a numeric code
                unicodeBuff[unicodeLen] = c;
                ++unicodeLen;
                if(unicodeLen == 4) {
                    unicodeBuff[4] = 0;
                    int charCode;
                    sscanf(unicodeBuff,"%04X",&charCode);
                    if(0x00 <= charCode && charCode <= 0x1F) {
                        ret.push_back(static_cast<char>(charCode));
                    }
                    state = 0;
                }
                break;
            case 0:
                if(c == '\\') {
                    state = 1;
                } else {
                    found = (ends.find(c) != std::string::npos);
                    if(!found) {
                        ret.push_back(c);
                    }
                }
                break;
        }
    } while( !found );
    return ret;
}

bool parseString(std::istream& stream, std::string & s) {
    s.clear();
    char c = lookUpAndEat(stream, "\"n");
    if(c == 'n') {
        lookUpTokenRemainder(stream, c, "null");
        return false;
    }
    s = lookUp(stream, "\"");
    return true;
}

bool parseBool(std::istream& stream, bool & b) {
    char c = lookUpAndEat(stream, "ntf");
    switch(c) {
        case 'n':
            lookUpTokenRemainder(stream, c, "null");
            return false;
        case 't':
            lookUpTokenRemainder(stream, c, "true");
            b = true;
            return true;
        case 'f':
            lookUpTokenRemainder(stream, c, "false");
            b = false;
            return true;
    }
    throw std::runtime_error("Bad boolean representation.");
}

bool lookUpNull(std::istream& stream) {
    char c = 0;
    do {
        c = stream.get();
        if(stream.bad()) {
            throw std::runtime_error("Encountered premature EOF");
        }
        if(!isblank(c) && c != '\n' && c != '\r') {
            if(c == 'n') {
                lookUpTokenRemainder(stream, c, "null");
                return true;
            } else {
                stream.putback(c);
                return false;
            }
        }
    } while( 1 );
    return false;
}

bool lookUpEmptyArray(std::istream& stream) {
    char c = 0;
    do {
        c = stream.get();
        if(stream.bad()) {
            throw std::runtime_error("Encountered premature EOF");
        }
        if(!isblank(c) && c != '\n' && c != '\r') {
            if(c == ']') {
                return true;
            } else {
                stream.putback(c);
                return false;
            }
        }
    } while( 1 );
    return false;
}

} /*namespace detail */

void serializeVector(std::ostream &stream, const std::vector<std::string> &v) {
    stream << "[";
    if(!v.empty()) {
        for(unsigned int i = 0; i < v.size() - 1; ++i) {
            auto & item = v.at(i);
            stream << "\"" << ll1gen::json::detail::encodeJsonString(item) << "\"" << ",";
        }
        stream << "\"" << ll1gen::json::detail::encodeJsonString(v.at(v.size() - 1)) << "\"";
    }
    stream << "]";
}

void deserializeVector(std::istream & stream, std::vector<std::string> &v) {
    v.clear();
    ll1gen::json::detail::lookUpAndEat(stream, "[");
    if(!ll1gen::json::detail::lookUpEmptyArray(stream)) {
        while(1) {
            std::string item;
            if(!ll1gen::json::detail::parseString(stream, item)) {
                throw new std::runtime_error("JSON: null not supported for vector/string. Check definition.");
            }
            v.push_back(std::move(item));
            char nextChar = ll1gen::json::detail::lookUpAndEat(stream, ",]");
            if(nextChar == ']') { break; }
        }
    }
}

void serializeVector(std::ostream &stream, const std::vector<bool> &v) {
    stream << "[";
    if(!v.empty()) {
        for(unsigned int i = 0; i < v.size() - 1; ++i) {
            stream << (v.at(i) ? "true" : "false") << ",";
        }
        stream << (v.at(v.size() - 1) ? "true" : "false");
    }
    stream << "]";
}

void deserializeVector(std::istream &stream, std::vector<bool> &v) {
    v.clear();
    ll1gen::json::detail::lookUpAndEat(stream, "[");
    if(!ll1gen::json::detail::lookUpEmptyArray(stream)) {
        while(1) {
            bool item;
            if(!ll1gen::json::detail::parseBool(stream, item)) {
                throw std::runtime_error("JSON: null is not supported for vector/bool. Check definition.");
            }
            v.push_back(item);
            char nextChar = ll1gen::json::detail::lookUpAndEat(stream, ",]");
            if(nextChar == ']') { break; }
        }
    }
}

template <typename T>
void serializeVector(std::ostream & stream, const std::vector<T> & v) {
    stream << "[";
    if(!v.empty()) {
        for(unsigned int i = 0; i < v.size() - 1; ++i) {
            auto & item = v.at(i);
            stream << item;
            stream << ",";
        }
        stream << v.at(v.size() - 1);
    }
    stream << "]";
}

template <typename T>
void deserializeVector(std::istream & stream, std::vector<T> & v) {
    v.clear();
    ll1gen::json::detail::lookUpAndEat(stream, "[");
    if(!ll1gen::json::detail::lookUpEmptyArray(stream)) {
        while(1) {
            T item;
            if(!ll1gen::json::detail::lookUpNull(stream)) {
                stream >> item;
            } else {
                throw std::runtime_error("JSON: null not supported for vector. Check definition");
            }
            v.push_back(std::move(item));
            char nextChar = ll1gen::json::detail::lookUpAndEat(stream, ",]");
            if(nextChar == ']') { break; }
        }
    }
}

void serializeRefVector(std::ostream &stream, const std::vector<std::shared_ptr<std::string>> &v) {
    stream << "[";
    if(!v.empty()) {
        for(unsigned int i = 0; i < v.size() - 1; ++i) {
            auto & item = v.at(i);
            if(item.get()) {
                stream << "\"" << ll1gen::json::detail::encodeJsonString(*item) << "\"";
            } else {
                stream << "null";
            }
            stream << ",";
        }
        auto & item = v.at(v.size() - 1);
        if(item.get()) {
            stream << "\"" << ll1gen::json::detail::encodeJsonString(*item) << "\"";
        } else {
            stream << "null";
        }
    }
    stream << "]";
}

void deserializeRefVector(std::istream &stream, std::vector<std::shared_ptr<std::string>> &v) {
    v.clear();
    ll1gen::json::detail::lookUpAndEat(stream, "[");
    if(!ll1gen::json::detail::lookUpEmptyArray(stream)) {
        while(1) {
            std::shared_ptr<std::string> item = std::make_shared<std::string>();
            if(!ll1gen::json::detail::lookUpNull(stream)) {
                ll1gen::json::detail::parseString(stream, *item);
            } else {
               item.reset();
            }
            v.push_back(std::move(item));
            char nextChar = ll1gen::json::detail::lookUpAndEat(stream, ",]");
            if(nextChar == ']') { break; }
        }
    }
}


void serializeRefVector(std::ostream &stream, const std::vector<std::shared_ptr<bool>> &v) {
    stream << "[";
    if(!v.empty()) {
        for(unsigned int i = 0; i < v.size() - 1; ++i) {
            auto & item = v.at(i);
            if(item.get()) {
                stream << ((*item) ? "true" : "false");
            } else {
                stream << "null";
            }
            stream << ",";
        }
        auto & item = v.at(v.size() - 1);
        if(item.get()) {
            stream << ((*item) ? "true" : "false");
        } else {
            stream << "null";
        }
    }
    stream << "]";
}

void deserializeRefVector(std::istream & stream, std::vector<std::shared_ptr<bool>> & v) {
    v.clear();
    ll1gen::json::detail::lookUpAndEat(stream, "[");
    if(!ll1gen::json::detail::lookUpEmptyArray(stream)) {
        while(1) {
            std::shared_ptr<bool> item = std::make_shared<bool>();
            if(!ll1gen::json::detail::lookUpNull(stream)) {
                ll1gen::json::detail::parseBool(stream, *item);
            } else {
               item.reset();
            }
            v.push_back(item);
            char nextChar = ll1gen::json::detail::lookUpAndEat(stream, ",]");
            if(nextChar == ']') { break; }
        }
    }
}

template <typename T>
void serializeRefVector(std::ostream & stream, const std::vector<std::shared_ptr<T>> & v) {
    stream << "[";
    if(!v.empty()) {
        for(unsigned int i = 0; i < v.size() - 1; ++i) {
            auto & item = v.at(i);
            if(item.get()) {
                stream << *item;
            } else {
                stream << "null";
            }
            stream << ",";
        }
        auto & item = v.at(v.size() - 1);
        if(item.get()) {
            stream << *item;
        } else {
            stream << "null";
        }
    }
    stream << "]";
}

template <typename T>
void deserializeRefVector(std::istream & stream, std::vector<std::shared_ptr<T>> & v) {
    v.clear();
    ll1gen::json::detail::lookUpAndEat(stream, "[");
    if(!ll1gen::json::detail::lookUpEmptyArray(stream)) {
        while(1) {
            std::shared_ptr<T> item = std::make_shared<T>();
            if(!ll1gen::json::detail::lookUpNull(stream)) {
                stream >> *item;
            } else {
               item.reset();
            }
            v.push_back(std::move(item));
            char nextChar = ll1gen::json::detail::lookUpAndEat(stream, ",]");
            if(nextChar == ']') { break; }
        }
    }
}


} }

#endif
