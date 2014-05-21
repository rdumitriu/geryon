
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

int main(int argn, const char * argv []) {
    geryon::util::Log::configureBasic(geryon::util::Log::DEBUG);
    test1();
    return 0;
}
