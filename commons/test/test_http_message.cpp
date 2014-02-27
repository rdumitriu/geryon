#include <thread>
#include <vector>
#include <iostream>

#include "http_types.hpp"

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
    }
};

int main(int argn, const char * argv []) {
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


    return 0;
}
