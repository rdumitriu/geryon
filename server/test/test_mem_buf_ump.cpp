
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
    if(buff.marker()) {
        LOG(geryon::util::Log::ERROR) << "ERROR: marker";
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

int main(int argn, const char * argv []) {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, MAX_BUFFS);

    if(argn == 1 || argv[1][0] == '1') {
        test1(pool);
    }

    return 0;
}
