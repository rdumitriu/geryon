
#include <vector>
#include <iostream>

#include "mem_buf_output_stream.hpp"

#include "log.hpp"

#define BUF_SZ 1024

class NoTCPPH : public geryon::server::TCPProtocolHandler {
public:
    explicit NoTCPPH(geryon::server::GMemoryPool * const _pMemoryPool) : geryon::server::TCPProtocolHandler(_pMemoryPool) {}
    virtual ~NoTCPPH() {}

    virtual void handleRead(geryon::server::GBufferHandler && currentBuffer, std::size_t nBytes) {} //does nothing

    /// \brief Writes the data on the wire (asynchronously).
    virtual void requestWrite(geryon::server::GBufferHandler && writeBuffer) {
        //std::cout << "Write called!" << std::endl;
        buffers.push_back(std::move(writeBuffer));
    }

    std::vector<geryon::server::GBufferHandler> buffers;
};

void test1() {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 0, 0);

    NoTCPPH protoHandler(&pool);
    //1st test : abcde
    {
        geryon::server::GOstreambuff buff1(&protoHandler);
        std::ostream oss1(&buff1);
        std::string s = "abcde";
        oss1 << s; //our stream;
        oss1.flush();
    }
    if(protoHandler.buffers.size() != 1) {
        LOG(geryon::util::Log::ERROR) << "ERROR: test1 failed (size).";
        return;
    }
    if(protoHandler.buffers.at(0).get().marker() != 5) {
        LOG(geryon::util::Log::ERROR) << "ERROR: test1 failed (size buff).";
    }
    if(strncmp(protoHandler.buffers.at(0).get().buffer(), "abcde", 5) != 0) {
        LOG(geryon::util::Log::ERROR) << "ERROR: test1 failed (content buff).";
    }
}

void checkFullBuffer(NoTCPPH * ph, unsigned int ndx, char ch) {
    if(ph->buffers.at(ndx).get().marker() != BUF_SZ) {
        LOG(geryon::util::Log::ERROR) << "ERROR: test2 failed (size buff). Index:" << ndx;
    }
    for(unsigned int i = 0; i < BUF_SZ; ++i) {
        char c = ph->buffers.at(ndx).get().buffer()[i];
        if(ch != c) {
            LOG(geryon::util::Log::ERROR) << "ERROR: test2 failed (content buff). Indexes:" << ndx << ", " << i;
        }
    }
}

void test2() {
    geryon::server::GUniformMemoryPool pool(BUF_SZ, 0, 0);

    NoTCPPH protoHandler(&pool);

    geryon::server::GOstreambuff buff1(&protoHandler);
    std::ostream oss1(&buff1);
    for(unsigned int i = 0; i < BUF_SZ / 4; ++i) {
        oss1 << "aaaa";
    }
    for(unsigned int i = 0; i < BUF_SZ / 2; ++i) {
        oss1 << "bb";
    }
    for(unsigned int i = 0; i < BUF_SZ; ++i) {
        oss1 << "c";
    }
    if(protoHandler.buffers.size() != 3) {
        LOG(geryon::util::Log::ERROR) << "ERROR: test2 failed (size).";
        return;
    }
    checkFullBuffer(&protoHandler, 0, 'a');
    checkFullBuffer(&protoHandler, 1, 'b');
    checkFullBuffer(&protoHandler, 2, 'c');
}

int main(int argn, const char * argv []) {
    geryon::util::Log::configureBasic(geryon::util::Log::DEBUG);
    test1();
    test2();
    return 0;
}
