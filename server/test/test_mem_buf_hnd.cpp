
#include "mem_buf.hpp"
#include "log.hpp"

#define BUF_SZ 1024

geryon::server::GBufferHandler && fct(geryon::server::GBufferHandler && o) {
    return std::move(o);
}

int main(int argn, const char * argv []) {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 1, 1);

    { //block start
        geryon::server::GBufferHandler hb1(&pool);
        if(!hb1.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (1)";
        }
    }
    { //block start
        geryon::server::GBufferHandler hb2(&pool, 0, false);
        if(!hb2.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (2)";
        }
    }

    { //block start
        geryon::server::GBufferHandler hb1(&pool, 0, false);
        geryon::server::GBufferHandler hb2(&pool, 0, false);
        try {
            hb1.get();
            hb2.get();
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (3)";
        } catch(std::bad_alloc &err) {} //do nothing

        if(!hb1.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (4)";
        }
        geryon::server::GBuffer buffer = hb1.get();

        //test move semantics
        geryon::server::GBufferHandler hb3(std::move(hb1));
        if(!hb3.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (5)";
        }

        if(buffer.buffer() != hb3.get().buffer()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (6)";
        }
        hb1 = std::move(hb3); //move it back
        if(!hb1.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (7)";
        }

        if(buffer.buffer() != hb1.get().buffer()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (8)";
        }
        try {
            hb3.get();
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (9)";
        } catch( ... ) {}
    }

    { //block start
        geryon::server::GBufferHandler hb1(&pool, 0, false);
        geryon::server::GBufferHandler hb2(&pool); //acquire
        if(!hb2.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (10)";
        }
        hb1 = fct(std::move(hb2));
        if(!hb1.get().isValid()) {
            LOG(geryon::util::Log::ERROR) << "ERROR: bad handler block (11)";
        }
    }


    return 0;
}
