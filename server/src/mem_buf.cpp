
#include <thread>
#include <chrono>

#include "mem_buf.hpp"
#include "log.hpp"

namespace geryon { namespace server {

GUniformMemoryPool::GUniformMemoryPool(std::size_t _bufferSize, std::size_t _initialNoBuf, std::size_t _maxNoBuf) :
            bufferSize(_bufferSize), maxNoBuf(_maxNoBuf) {
    if(maxNoBuf < _initialNoBuf) {
        maxNoBuf = _initialNoBuf;
    }
    for(unsigned int i = 0; i < _initialNoBuf; i++) {
        char * allocated = internalAlloc();
        if(allocated) {
            readyBuffers.push_back(allocated);
        }
    }
    LOG(geryon::util::Log::INFO) << "Initialized pool of " << (bufferSize / 1024) << "K blocks"
                                 << ", minimal size: " << ((bufferSize / 1024) * _initialNoBuf) << "K"
                                 << ", maximal size: " << ((bufferSize / 1024) * maxNoBuf) << "K";
}

GUniformMemoryPool::~GUniformMemoryPool() {
    for(auto pa : readyBuffers) {
        delete [] (pa);
    }
    //not all buffers returned to the pool ?!? we're screwed
    if(borrowedBuffers.size() > 0) {
        LOG(geryon::util::Log::ERROR) << "Internal allocation problem detected in the memory pool of "
                                      << (bufferSize / 1024) << "K blocks. Not all blocks are in the ready list.";
    }
}

char * GUniformMemoryPool::internalAlloc() {
    char * p = 0;
    try {
        p = new char[bufferSize];
    } catch ( ... ) {
        LOG(geryon::util::Log::WARNING) << "Failed to allocate direct buffer into the memory pool of "
                                        << (bufferSize / 1024) << "K blocks.";
        //hide away alloc errors, we'll fail louder afterwards.
    }
    return p;
}

#define MAX_ALLOC_SPINS 128
#define ALLOC_SPIN_TRESHOLD 16
#define ALLOC_YIELD_DURATION 1

GBuffer GUniformMemoryPool::acquire() {
    unsigned int spins = 0;
    while(spins < MAX_ALLOC_SPINS) {
        //initial busy wait transforms into timed wait
        if(spins > ALLOC_SPIN_TRESHOLD) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ALLOC_YIELD_DURATION));
        }
        //block start, we make it explicit
        {
            std::unique_lock<std::mutex> _(mutex);
            //let's see if we have something ready
            if(readyBuffers.empty()) {
                //we're zero on ready buffers, so we must allocate
                //check to see if we must (kind-a) busy wait (maxed-out)
                if(maxNoBuf) {
                    std::size_t sz = readyBuffers.size() + borrowedBuffers.size();
                    if(sz == maxNoBuf) {
                        ++spins;
                        LOG(geryon::util::Log::WARNING) << "Memory pool of " << (bufferSize / 1024)
                                                        << "K blocks is maxed-out. Retrying.";
                        continue;
                    }
                }
                //not maxed out, we're allowed to allocate
                char * head = internalAlloc();
                if(!head) { //shit!
                    ++spins;
                    continue;
                }
                borrowedBuffers.insert(head);
                return GBuffer(bufferSize, head, 0);
            } else {
                //we have something ready
                char * head = readyBuffers.front();
                readyBuffers.pop_front();
                borrowedBuffers.insert(head);
                return GBuffer(bufferSize, head, 0);
            }
        }
        //block end
        ++spins;
    } //while
    LOG(geryon::util::Log::FATAL) << "Failed to allocate memory into the memory pool of "
                                  << (bufferSize / 1024) << "K blocks.";
    throw std::bad_alloc();
}

void GUniformMemoryPool::release(const GBuffer & pab) {
    if(pab.size() != bufferSize) {
        LOG(geryon::util::Log::WARNING) << "Buffer does not belong to this pool. Internal error. Memory pool: "
                                        << (bufferSize / 1024) << "K blocks, block size:" << pab.size();
        return; //will leak
    }
    if(!pab.buffer()) {
        LOG(geryon::util::Log::WARNING) << "NULL buffer passed on. Internal error. Memory pool: "
                                        << (bufferSize / 1024) << "K blocks, block size:" << pab.size();
        return; //will leak
    }
    std::unique_lock<std::mutex> _(mutex);
    char * buff = pab.buffer();
    if(!borrowedBuffers.erase(buff)) {
        LOG(geryon::util::Log::WARNING) << "Buffer not found into the borrowed ones. Internal error. Memory pool: "
                                        << (bufferSize / 1024) << "K blocks.";
    }
    readyBuffers.push_back(buff);
}

} }

