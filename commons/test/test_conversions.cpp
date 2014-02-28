#include <thread>
#include <vector>
#include <iostream>

#include "string_utils.hpp"

using namespace std;
using namespace geryon::util;



int main(int argn, const char * argv []) {
    int i = convertTo("123", 0);
    if(i != 123) {
        cout << "FAILED : int" << endl;
    }
    unsigned long sz = convertTo("123456", 0UL);
    if(sz != 123456) {
        cout << "FAILED : UL" << endl;
    }
    cout << "i=" << i << " - sz=" << sz << endl;

    string dt("2014-Feb-02 12:00:01");
    time_t t = convertISODateTime(dt);
    cout << "t=" << t << " local:" << ctime(&t) << endl; //not thread-safe
    string dt2 = formatISODateTime(t);
    if(dt != dt2) {
        cout << "FAILED : " << dt << " != " << dt2 << endl;
    }

    return 0;
}
