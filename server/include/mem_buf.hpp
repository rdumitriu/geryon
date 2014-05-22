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
    GBuffer(std::size_t _size = 0, char * _buffer = 0, std::size_t _marker = 0) :
                    sz(_size), buff(_buffer), mrk(_marker) {}
    ~GBuffer() {}

    inline std::size_t size() const { return sz; }
    inline std::size_t marker() const { return mrk; }
    inline char * buffer() const { return buff; }
    inline bool isValid() { return (buff != 0 && sz != 0); }

    inline void advanceMarker(std::size_t amount) {
        std::size_t x = mrk + amount;
        if(x > sz) {
            throw std::invalid_argument("Cannot move marker past the size of the buffer!");
        }
        mrk = x;
    }

    inline void setMarker(std::size_t _marker) {
        if(_marker > sz) {
            throw std::invalid_argument("Cannot set marker past the size of the buffer!");
        }
        mrk = _marker;
    }

private:
    std::size_t sz;
    char * buff;
    std::size_t mrk;
};

#define G_MEMORY_POOL_MIN_BUFFER_SIZE 1024
#define G_MEMORY_POOL_PAGE_SIZE 1024

///
/// \brief The GMemoryPool is the interface to different strategies of allocation
/// /// ::TODO:: soon, we'll have to create a deallocator.
///
class GMemoryPool {
public:
    GMemoryPool(std::size_t _bufferSize) : bufferSize(std::max(_bufferSize, static_cast<std::size_t>(G_MEMORY_POOL_MIN_BUFFER_SIZE))) {
        //adjust
        if(bufferSize % G_MEMORY_POOL_PAGE_SIZE != 0) {
            bufferSize = (bufferSize / G_MEMORY_POOL_PAGE_SIZE + 1) * G_MEMORY_POOL_PAGE_SIZE;
        }
        bufferSizeK = bufferSize / 1024;
    }
    virtual ~GMemoryPool() {}

    ///
    /// \brief Acquire a buffer
    ///
    /// \param hint the quantity needed, if you know it, zero if you're clueless
    ///
    /// Acquire a buffer. By contract, if the buffer cannot be aquired, it should throw std::bad_alloc
    ///
    virtual GBuffer acquire(std::size_t hint = 0) = 0;

    ///
    /// \brief Release the buffer
    ///
    virtual void release(const GBuffer & pab) = 0;

    ///\brief Gets the buffer size, in bytes
    inline std::size_t getBufferSize() const { return bufferSize; }

    ///\brief Gets the buffer size, in bytes
    inline std::size_t getBufferSizeK() const { return bufferSizeK; }

    ///\brief Gets the buffer size, in kilobytes
    virtual std::size_t getAllocatedSizeK() = 0;

    ///\brief Gets the buffer size, in kilobytes
    virtual std::size_t getMaxSizeK() const = 0;
private:
    std::size_t bufferSize;
    std::size_t bufferSizeK;
};

///
/// \brief The GUniformMemoryPool class
///
/// The memory pool which has a uniform size for all the buffers.
///
class GUniformMemoryPool : public GMemoryPool {
public:
    explicit GUniformMemoryPool(std::size_t _bufferSize, std::size_t _initialNoBuf = 0, std::size_t _maxNoBuf = 0);
    virtual ~GUniformMemoryPool();

    ///\brief Acquire a buffer
    virtual GBuffer acquire(std::size_t hint = 0);

    ///\brief Release the buffer
    virtual void release(const GBuffer & pab);

    ///
    /// \brief Gets the max size, in number of buffers.
    ///
    /// Gets the max size. Zero means unlimited.
    inline std::size_t getMaxBuffers() const { return maxNoBuf; }

    ///\brief Gets the buffer size, in kilobytes
    virtual std::size_t getAllocatedSizeK();

    ///\brief Gets the buffer size, in kilobytes
    virtual std::size_t getMaxSizeK() const;
private:
    std::size_t bufferSize;
    std::size_t maxNoBuf;
    std::deque<char *> readyBuffers;
    std::set<char *> borrowedBuffers;
    std::mutex mutex;

    char * internalAlloc();
};

///
/// \brief The GBufferHandler class
///
/// The handler to our buffers. The preferred way to deal with them.
/// It is movable, but not copyable.
///
class GBufferHandler {
public:
    explicit GBufferHandler(GMemoryPool * const _addr = 0, std::size_t hint = 0, bool acquire = true) : pGPool(_addr) {
        if(pGPool && acquire) {
           buffer = pGPool->acquire(hint);
        }
    }

    ~GBufferHandler() {
        if(pGPool && buffer.isValid()) {
            pGPool->release(buffer);
        }
    }

    ///
    /// \brief Gets the buffer.
    /// \param hint the hint
    /// \return the valid buffer
    /// May throw bad_alloc if buffer cannot be allocated
    ///
    GBuffer & get(std::size_t hint = 0) {
        if(!pGPool) {
            throw std::runtime_error("Usage of uninitialized memory handler!");
        }
        if(!buffer.isValid()) {
            buffer = pGPool->acquire(hint);
        }
        return buffer;
    }

    ///
    /// \brief Is this handler valid ?
    /// \return true if valid
    ///
    bool isValid() {
        return (pGPool && buffer.isValid());
    }

    GBufferHandler& operator= (GBufferHandler&& other) {
        if(this != &other) {
            if(pGPool && buffer.isValid()) {
                pGPool->release(buffer);
            }
            this->buffer = other.buffer;
            this->pGPool = other.pGPool;
            other.pGPool = 0; //steal the pool pointer
        }
        return (*this);
    }

    GBufferHandler (GBufferHandler&& other) {
        buffer = other.buffer;
        pGPool = other.pGPool;
        other.pGPool = 0; //steal the pool pointer
    }

    GBufferHandler(const GBufferHandler&) = delete; // prevent copy constructor to be used
    GBufferHandler& operator= (const GBufferHandler&) = delete; // prevent copy assignment to be used

private:
    GMemoryPool * pGPool;
    GBuffer buffer;
};


} }

#endif
