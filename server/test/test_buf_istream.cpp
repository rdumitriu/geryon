
#include <iostream>

#include "mem_buf_input_stream.hpp"
#include "log.hpp"

#define BUF_SZ 1024

void test1() {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, 0);
    std::vector<geryon::server::GBufferHandler> buffers;

    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(0).get().buffer()[0] = 'a'; //0
    buffers.at(0).get().buffer()[1] = 'a'; //1
    buffers.at(0).get().setMarker(2);

    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(1).get().buffer()[0] = 'b'; //2
    buffers.at(1).get().buffer()[1] = 'b'; //3
    buffers.at(1).get().buffer()[2] = 'b'; //4
    buffers.at(1).get().setMarker(3);

    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(2).get().buffer()[0] = 'c'; //5
    buffers.at(2).get().buffer()[1] = 'c'; //6
    buffers.at(2).get().buffer()[2] = 'c'; //7
    buffers.at(2).get().buffer()[3] = 'c'; //8
    buffers.at(2).get().setMarker(4);

    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(3).get().buffer()[0] = 'd'; //9
    buffers.at(3).get().setMarker(1);

    //1st test : abbbc
    geryon::server::GIstreambuff buff1(buffers, 1, 6);
    std::istream iss1(&buff1);
    std::string s;
    iss1 >> s; //our stream
    if("abbbc" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed (1)";
    }
    std::cout << "String 1 is >>" << s << "<<" << std::endl;

    //2nd test: all aabbbccccd
    geryon::server::GIstreambuff buff2(buffers, 0, 10);
    std::istream iss2(&buff2);
    iss2 >> s; //our stream
    if("aabbbccccd" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed (2)";
    }
    std::cout << "String 2 is >>" << s << "<<" << std::endl;

    //3nd test: empty (EOF)
    geryon::server::GIstreambuff buff3(buffers, 1, 1);
    std::istream iss3(&buff3);
    if(iss3 >> s) { //yeap, this should return false
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed (3)";
    }
    std::cout << "String 3 is >>" << s << "<<" << std::endl; //unchanged
}

int main(int argn, const char * argv []) {
    test1();
    return 0;
}
