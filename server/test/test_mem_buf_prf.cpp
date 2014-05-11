
#include <chrono>

#include "mem_buf.hpp"
#include "log.hpp"

#define BUF_SZ (16 * 1024)

#define CYCLES 100000

void test1(geryon::server::GUniformMemoryPool * ppool) {
    geryon::server::GBufferHandler handlers[CYCLES];
    for(unsigned int i = 0; i < CYCLES; i++) {
        handlers[i] = geryon::server::GBufferHandler(ppool);
    }
}

typedef char * pchar;
pchar pointers[CYCLES];

void test2() {
    for(unsigned int i = 0; i < CYCLES; i++) {
        pointers[i] = new char[BUF_SZ];
    }
    for(unsigned int i = 0; i < CYCLES; i++) {
        delete [] pointers[i];
    }

}

int main(int argn, const char * argv []) {


    std::chrono::milliseconds d1;

    {
        geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, 0);
        std::chrono::time_point<std::chrono::system_clock> tm1 = std::chrono::system_clock::now();
        test1(&pool);
        test1(&pool); //re-use
        std::chrono::time_point<std::chrono::system_clock> tm2 = std::chrono::system_clock::now();
        d1 = std::chrono::milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(tm2 - tm1));
    }

    std::chrono::time_point<std::chrono::system_clock> tm3 = std::chrono::system_clock::now();
    test2();
    test2(); //re-alloc
    std::chrono::time_point<std::chrono::system_clock> tm4 = std::chrono::system_clock::now();


    std::chrono::milliseconds d2 = std::chrono::milliseconds(std::chrono::duration_cast<std::chrono::milliseconds>(tm4 - tm3));
    LOG(geryon::util::Log::INFO) << d1.count() << " - " << d2.count();
    return 0;
}
