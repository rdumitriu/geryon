#include <thread>
#include <vector>
#include <iostream>

#include "http_types.hpp"
#include "log.hpp"

using namespace std;
using namespace geryon;

class TestBase : public HttpMessage {
public:
    TestBase() : HttpMessage() {
        HttpHeader h;
        h.name = "THeader";
        h.values.push_back("THeaderValue");

        headers.insert(std::make_pair(h.name, h));
        contentLength = 1234;
        contentType = "bogus";

        //cookie fun
        HttpHeader hc;
        hc.name = "Cookie";
        hc.values.push_back("firstcookie=acc=1&lgn=user; path=/path/to/cookie/heaven; httpOnly");
        hc.values.push_back("anothercookie=value; path=/");
        headers.insert(std::make_pair(hc.name, hc));
    }
};

int main(int argn, const char * argv []) {
    geryon::util::Log::configureBasic(geryon::util::Log::DEBUG);

    TestBase test;

    cout << "Testing the base behaviour" << endl;
    if(test.getContentLength() != 1234) { cout << "FAILED: content length" << endl; }
    if(test.getContentType() != "bogus") { cout << "FAILED: content type" << endl; }
    if(!test.hasHeader("THeader")) {
        cout << "FAILED : no header, but expected to be" << endl;
    } else {
        string s = test.getHeaderValue("THeader");
        if(s != "THeaderValue") {
            cout << "FAILED : found header, but bad value:" << s << endl;
        }
        vector<string> vals = test.getHeaderValues("THeader");
        if(vals.size() != 1) {
            cout << "FAILED : found header, but bad values size:" << vals.size() << endl;
        }
        if(vals[0] != "THeaderValue") {
            cout << "FAILED : found header, but bad value:" << vals[0] << endl;
        }
    }
    cout << "Testing the attributes" << endl;
    long x = 12345;
    long y;
    test.putAttribute("long", x);
    if(!test.getAttribute("long", y)) {
        cout << "FAILED : basic attrs, getting it failed!" << endl;
    } else {
        if(x != y) {
            cout << "FAILED : basic attrs, getting it failed! Value = " << y << endl;
        }
    }
    if(!test.removeAttribute("long")) {
        cout << "FAILED : basic attrs, removing it failed!" << endl;
    }
    if(test.getAttribute("long", y)) {
        cout << "FAILED : basic attrs, should not be here!" << endl;
    }

    //cookie test
    HttpCookie c1;
    if(!test.hasCookie("firstcookie")) {
        cout << "FAILED : no 1st cookie ?!? (1)" << endl;
    }
    cout << "Getting cookie 1" << endl;
    if(!test.getCookie("firstcookie", c1)) {
        cout << "FAILED : no 1st cookie ?!? (2)" << endl;
    } else {
        if(c1.value != "acc=1&lgn=user" ) {
            cout << "FAILED : 1st value (3) :" << c1.value << endl;
        }
        if(!c1.httpOnly) {
            cout << "FAILED : 1st value (4)" << endl;
        }
        if(c1.path != "/path/to/cookie/heaven") {
            cout << "FAILED : 1st value (5) :" << c1.path << endl;
        }
    }
    if(!test.getCookie("anothercookie", c1)) {
        cout << "FAILED : no 2nd cookie ?!?" << endl;
    }

    return 0;
}
