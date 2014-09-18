
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

    //3rd test: reset the stream
    buff2.setup(1,9);
    std::istream iss2_1(&buff2); //our stream
    iss2_1 >> s;
    if("abbbcccc" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed (3.1)";
    }
    std::cout << "String 3.1 is >>" << s << "<<" << std::endl;
    buff2.setup(2,8);
    iss2.rdbuf(&buff2);
    iss2 >> s;
    if("bbbccc" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed (3.2)";
    }
    std::cout << "String 3.2 is >>" << s << "<<" << std::endl;


    //4th test: empty (EOF)
    geryon::server::GIstreambuff buff3(buffers, 0, 0);
    std::istream iss3(&buff3);
    if(iss3 >> s) { //yeap, this should return false
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed (4)";
    }
    std::cout << "String 4 is >>" << s << "<<" << std::endl; //unchanged
}

void test2() {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, 0);
    std::vector<geryon::server::GBufferHandler> buffers;

    geryon::server::GIstreambuff buff1(buffers, 0, 0);
    std::istream iss1(&buff1);
}

void test1Gaps() {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, 0);
    std::vector<geryon::server::GBufferHandler> buffers;
    
    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(0).get().buffer()[0] = 'a'; //0
    buffers.at(0).get().buffer()[1] = 'X'; //1
    buffers.at(0).get().buffer()[2] = 'a'; //2
    buffers.at(0).get().setMarker(3);
    
    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(1).get().buffer()[0] = 'b'; //3
    buffers.at(1).get().buffer()[1] = 'b'; //4
    buffers.at(1).get().buffer()[2] = 'X'; //5
    buffers.at(1).get().buffer()[3] = 'X'; //6
    buffers.at(1).get().buffer()[4] = 'b'; //7
    buffers.at(1).get().setMarker(5);
    
    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(2).get().buffer()[0] = 'c'; //8
    buffers.at(2).get().buffer()[1] = 'c'; //9
    buffers.at(2).get().buffer()[2] = 'c'; //10
    buffers.at(2).get().buffer()[3] = 'X'; //11
    buffers.at(2).get().setMarker(4);
    
    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(3).get().buffer()[0] = 'X'; //12
    buffers.at(3).get().buffer()[1] = 'd'; //13
    buffers.at(3).get().buffer()[2] = 'd'; //14
    buffers.at(3).get().buffer()[3] = 'X'; //15
    buffers.at(3).get().setMarker(4);
    
    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(4).get().buffer()[0] = 'X'; //16
    buffers.at(4).get().buffer()[1] = 'e'; //17
    buffers.at(4).get().buffer()[2] = 'e'; //18
    buffers.at(4).get().buffer()[3] = 'X'; //19
    buffers.at(4).get().setMarker(4);
    
    
    buffers.push_back(std::move(geryon::server::GBufferHandler(&pool)));
    buffers.at(5).get().buffer()[0] = 'X'; //19
    buffers.at(5).get().setMarker(1);
    
    //1st test : abb
    geryon::server::GIstreambuff buff1(buffers, 1, 5);
    buff1.addGap(1, 2);
    std::istream iss1(&buff1);
    std::string s;
    iss1 >> s; //our stream
    if("abb" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed test1Gaps (1)";
    }
    std::cout << "String 1 is >>" << s << "<<" << std::endl;
    
    //2nd test: all abbbcccddee
    geryon::server::GIstreambuff buff2(buffers, 1, 20);
    buff2.addGap(1, 2);
    buff2.addGap(5, 7);
    buff2.addGap(11, 13);
    buff2.addGap(15, 17);
    buff2.addGap(19, 20);
    std::istream iss2(&buff2);
    iss2 >> s; //our stream
    if("abbbcccddee" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed test1Gaps (2)";
    }
    std::cout << "String 2 is >>" << s << "<<" << std::endl;
    
    //3rd test: reset the stream
    buff2.setup(2,4);
    std::istream iss2_1(&buff2); //our stream
    iss2_1 >> s;
    if("ab" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed test1Gaps (3.1)";
    }
    std::cout << "String 3.1 is >>" << s << "<<" << std::endl;
    buff2.setup(11,18);
    buff2.addGap(11, 13);
    buff2.addGap(15, 17);
    iss2.rdbuf(&buff2);
    iss2 >> s;
    if("dde" != s) {
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed test1Gaps (3.2)";
    }
    std::cout << "String 3.2 is >>" << s << "<<" << std::endl;
    
    
    //4th test: empty (EOF)
    geryon::server::GIstreambuff buff3(buffers, 11, 13);
    buff3.addGap(11, 13);
    std::istream iss3(&buff3);
    if(iss3 >> s) { //yeap, this should return false
        LOG(geryon::util::Log::ERROR) << "ERROR: Failed test1Gaps (4)";
    }
    std::cout << "String 4 is >>" << s << "<<" << std::endl; //unchanged
}

int main(int argn, const char * argv []) {
    test1();
    test2();
    test1Gaps();
    return 0;
}
