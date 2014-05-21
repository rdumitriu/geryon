
#include <thread>
#include <chrono>

#include "mem_buf.hpp"
#include "log.hpp"

namespace geryon { namespace server {

GUniformMemoryPool::GUniformMemoryPool(std::size_t _bufferSize, std::size_t _initialNoBuf, std::size_t _maxNoBuf) :
            GMemoryPool(_bufferSize), maxNoBuf(_maxNoBuf) {
    if(maxNoBuf < _initialNoBuf && maxNoBuf > 0) {
        maxNoBuf = _initialNoBuf;
    }
    for(unsigned int i = 0; i < _initialNoBuf; i++) {
        char * allocated = internalAlloc();
        if(allocated) {
            readyBuffers.push_back(allocated);
        }
    }
    LOG(geryon::util::Log::INFO) << "Initialized pool of " << getBufferSizeK() << "K blocks"
                                 << ", minimal size: " << (getBufferSizeK() * _initialNoBuf) << "K"
                                 << ", maximal size: " << (getBufferSizeK() * maxNoBuf) << "K";
}

GUniformMemoryPool::~GUniformMemoryPool() {
    for(auto pa : readyBuffers) {
        delete [] (pa);
    }
    LOG(geryon::util::Log::INFO) << "Deallocated pool of " << getBufferSizeK() << "K blocks"
                                 << ", actual size: " << (getBufferSizeK() * readyBuffers.size()) << "K";
    //not all buffers returned to the pool ?!? we're screwed
    if(borrowedBuffers.size() > 0) {
        LOG(geryon::util::Log::ERROR) << "Internal allocation problem detected in the memory pool of "
                                      << getBufferSizeK() << "K blocks. Not all blocks are in the ready list.";
    }
    //de-allocate them, even if wrong
    for(auto pa : borrowedBuffers) {
        delete [] (pa);
    }
}

char * GUniformMemoryPool::internalAlloc() {
    char * p = 0;
    try {
        p = new char[getBufferSize()];
    } catch ( ... ) {
        LOG(geryon::util::Log::WARNING) << "Failed to allocate direct buffer into the memory pool of "
                                        << getBufferSizeK() << "K blocks.";
        //hide away alloc errors, we'll fail louder afterwards.
    }
    return p;
}

#define MAX_ALLOC_SPINS 128
#define ALLOC_SPIN_TRESHOLD 16
#define ALLOC_YIELD_DURATION 1

GBuffer GUniformMemoryPool::acquire(std::size_t hint) {
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
                    if(borrowedBuffers.size() == maxNoBuf) {
                        ++spins;
                        continue;
                    }
                }
                //not maxed out, we're allowed to allocate
                char * head = internalAlloc();
                if(!head) { //shit! memory full!
                    ++spins;
                    continue;
                }
                borrowedBuffers.insert(head);
                LOG(geryon::util::Log::DEBUG) << "Allocating buffer:" << static_cast<void *>(head);
                return GBuffer(getBufferSize(), head);
            } else {
                //we have something ready
                char * head = readyBuffers.front();
                readyBuffers.pop_front();
                borrowedBuffers.insert(head);
                LOG(geryon::util::Log::DEBUG) << "Reusing buffer:" << static_cast<void *>(head);
                return GBuffer(getBufferSize(), head);
            }
        }
        //block end
        ++spins;
    } //while
    LOG(geryon::util::Log::FATAL) << "Failed to allocate memory into the memory pool of "
                                  << getBufferSizeK() << "K blocks.";
    if(maxNoBuf) {
        std::unique_lock<std::mutex> _(mutex); //protect this, we need the sizes
        std::size_t sz = readyBuffers.size() + borrowedBuffers.size();
        if(sz == maxNoBuf) {
            LOG(geryon::util::Log::FATAL) << "Memory pool of " << getBufferSizeK() << "K blocks is maxed-out.";
        } else {
            LOG(geryon::util::Log::FATAL) << "Cannot allocate memory, pool of " << getBufferSizeK() << "K blocks.";
        }
    } else {
        LOG(geryon::util::Log::FATAL) << "Cannot allocate memory, pool of " << getBufferSizeK() << "K blocks.";
    }
    throw std::bad_alloc();
}

void GUniformMemoryPool::release(const GBuffer & pab) {
    if(pab.size() != getBufferSize()) {
        LOG(geryon::util::Log::WARNING) << "Buffer does not belong to this pool. Internal error. Memory pool: "
                                        << getBufferSizeK() << "K blocks, block size:" << pab.size();
        return; //will leak
    }
    if(!pab.buffer()) {
        LOG(geryon::util::Log::WARNING) << "NULL buffer passed on. Internal error. Memory pool: "
                                        << getBufferSizeK() << "K blocks, block size:" << pab.size();
        return; //will not leak, but it's not good
    }
    std::unique_lock<std::mutex> _(mutex);
    char * buff = pab.buffer();
    if(!borrowedBuffers.erase(buff)) {
        LOG(geryon::util::Log::WARNING) << "Buffer not found into the borrowed ones. Internal error. Memory pool: "
                                        << getBufferSizeK() << "K blocks.";
    }
    LOG(geryon::util::Log::DEBUG) << "Releasing buffer:" << static_cast<void *>(buff);
    readyBuffers.push_back(buff);
}

std::size_t GUniformMemoryPool::getAllocatedSizeK() {
    std::size_t sz = 0;
    {
        std::unique_lock<std::mutex> _(mutex); //protect this, we need the sizes
        sz = readyBuffers.size() + borrowedBuffers.size();
    }
    return (getBufferSizeK() * sz);
}

std::size_t GUniformMemoryPool::getMaxSizeK() const {
    return (getBufferSizeK() * maxNoBuf); //no need to lock
}

} }

