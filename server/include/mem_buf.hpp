///
/// \file mem_buf.hpp
///
///  Created on: Apr 18, 2014
///      Author: rdumitriu at gmail.com

#ifndef MEM_BUF_HPP_
#define MEM_BUF_HPP_

#include <string>
#include <deque>
#include <set>
#include <mutex>

namespace geryon { namespace server {

///
/// \brief The GBuffer struct
///
/// It is cheap to copy, thus suitable for returns. Buffers data grow by adding into the buffer, therefore it has a
/// marker there to show the number of bytes filled in the buffer.
///
/// No alloc or dealloc is done. Pointer is copied as it is so you should take great care not to dealloc it twice.
/// In fact, you should never (ever!) dealloc the buffers, since they should be managed by the memory pools (see below)
///
/// This is not semantically equivalent with the asioBuffer (mutable_buffers), although it is very close to it
struct GBuffer {
public:
    GBuffer(std::size_t _size, char * _buffer, std::size_t _marker = 0) :
                    sz(_size), buff(_buffer), mrk(_marker) {}
    ~GBuffer() {}

    inline std::size_t size() const { return sz; }
    inline char * buffer() const { return buff; }
    inline std::size_t marker() const { return mrk; }
    inline void moveMarker(std::size_t newPos) { this->mrk = newPos; }
private:
    std::size_t sz;
    char * buff;
    std::size_t mrk;
};

///
/// \brief The GUniformMemoryPool class
///
/// The memory pool which has a uniform size for all the buffers.
/// ::TODO:: soon, we'll have to create a deallocator.
///
class GUniformMemoryPool {
public:
    explicit GUniformMemoryPool(std::size_t _bufferSize, std::size_t _initialNoBuf = 0, std::size_t _maxNoBuf = 0);
    virtual ~GUniformMemoryPool();

    //acquire a buffer
    GBuffer acquire();
    //release it
    void release(const GBuffer & pab);

private:
    std::size_t bufferSize;
    std::size_t maxNoBuf;
    std::deque<char *> readyBuffers;
    std::set<char *> borrowedBuffers;
    std::mutex mutex;

    char * internalAlloc();
};


} }

#endif
