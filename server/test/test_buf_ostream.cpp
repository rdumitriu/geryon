
#include <iostream>

#include "mem_buf_output_stream.hpp"
#include "log.hpp"

#define BUF_SZ 1024

void test1() {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 0, 0);

    //1st test : abbbc
    geryon::server::GOstreambuff buff1(&pool);
    std::ostream oss1(&buff1);
    std::string s = "abc";
    oss1 << s << 123; //our stream
    oss1.flush();
}

int main(int argn, const char * argv []) {
    geryon::util::Log::configureBasic(geryon::util::Log::DEBUG);
    test1();
    return 0;
}
