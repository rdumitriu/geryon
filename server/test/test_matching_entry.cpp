
#include "server_application_utils.hpp"
#include "log.hpp"


using namespace std;
using namespace geryon::server::detail;

void test1() {
    MatchingEntry me = createMatchingEntry("/some/path/*/to/file");
    if(me.beginOfPath != "/some/path/") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test1(1) failed: " << me.beginOfPath;
    }
    if(me.endOfPath != "/to/file") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test1(2) failed: " << me.endOfPath;
    }
    if(!isMatchingEntry("/some/path/aaabbb/to/file", me)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test1(3) failed: not matching";
    }
}

void test2() {
    MatchingEntry me = createMatchingEntry("/some/path/*");
    if(me.beginOfPath != "/some/path/") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test2(1) failed: " << me.beginOfPath;
    }
    if(me.endOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test2(2) failed: " << me.endOfPath;
    }
    if(!isMatchingEntry("/some/path/aaabbb/to/file", me)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test2(3) failed: not matching";
    }
}

void test3() {
    MatchingEntry me = createMatchingEntry("*");
    if(me.beginOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test3(1) failed: " << me.beginOfPath;
    }
    if(me.endOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test3(2) failed: " << me.endOfPath;
    }
    if(!isMatchingEntry("/some/path/aaabbb/to/file", me)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test3(3) failed: not matching";
    }
}

void test4() {
    MatchingEntry me = createMatchingEntry("/some*/path/*.do");
    if(me.beginOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test4(1) failed: " << me.beginOfPath;
    }
    if(me.endOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test4(2) failed: " << me.endOfPath;
    }
    if(!isMatchingEntry("/something/path/to.do", me)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test4(3) failed: not matching";
    }
}

void test5() {
    MatchingEntry me = createMatchingEntry("/some/path");
    if(me.beginOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test5(1) failed: " << me.beginOfPath;
    }
    if(me.endOfPath != "") {
        LOG(geryon::util::Log::ERROR) << "FAILED: test5(2) failed: " << me.endOfPath;
    }
    if(!isMatchingEntry("/some/path", me)) {
        LOG(geryon::util::Log::ERROR) << "FAILED: test5(3) failed: not matching";
    }
}

int main(int argn, const char * argv []) {
    test1();
    test2();
    test3();
    test4();
    return 0;
}
