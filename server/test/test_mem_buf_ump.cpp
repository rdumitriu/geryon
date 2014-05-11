
#include "mem_buf.hpp"
#include "log.hpp"

#define BUF_SZ 1024
#define MAX_BUFFS 3

//positive test
void test1(geryon::server::GUniformMemoryPool & pool) {
    geryon::server::GBuffer buff = pool.acquire();
    if(BUF_SZ != buff.size()) {
        LOG(geryon::util::Log::ERROR) << "ERROR: buffer size";
    }
    if(!buff.buffer()) {
        LOG(geryon::util::Log::ERROR) << "ERROR: buffer itself";
    }
    char * pointer = buff.buffer();
    pool.release(buff);


    buff = pool.acquire();
    if(pointer != buff.buffer()) {
        LOG(geryon::util::Log::ERROR) << "ERROR: not reused!";
    }
    geryon::server::GBuffer buff2 = pool.acquire();
    geryon::server::GBuffer buff3 = pool.acquire();
    //this must alloc new buff
    if(buff.buffer() == buff2.buffer() || buff.buffer() == buff3.buffer() || buff2.buffer() == buff3.buffer()) {
        LOG(geryon::util::Log::ERROR) << "ERROR: really bad alloc";
    }
    if(buff.size() != buff2.size() || buff.size() != buff3.size()) {
        LOG(geryon::util::Log::ERROR) << "ERROR: bad sizes";
    }
    pool.release(buff);
    pool.release(buff2);
    pool.release(buff3);
}

//positive test
void test2(geryon::server::GUniformMemoryPool & pool) {
    geryon::server::GBuffer buffs[MAX_BUFFS];
    for(int i = 0; i < MAX_BUFFS; i++) {
        buffs[i] = pool.acquire();
    }
    //we have everything allocated, so let's try to allocate more
    try {
        geryon::server::GBuffer neverbe = pool.acquire();
        LOG(geryon::util::Log::ERROR) << "ERROR: should not reach here! (1)";
    } catch(std::bad_alloc & err) {
        LOG(geryon::util::Log::INFO) << "Good, we failed correctly.";
    }
    for(int i = 0; i < MAX_BUFFS; i++) {
         pool.release(buffs[i]);
    }
}

int main(int argn, const char * argv []) {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, MAX_BUFFS);

    if(argn == 1 || argv[1][0] == '1') {
        test1(pool);
    } else if(argv[1][0] == '2') {
        test2(pool);
    }

    return 0;
}
