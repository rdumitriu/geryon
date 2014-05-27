
#include "mem_buf_input_stream.hpp"

namespace geryon { namespace server {


GIstreambuff::GIstreambuff(std::vector<GBufferHandler> & _buffers,
                           std::size_t _begin, std::size_t _end)
                                        : std::streambuf(), buffers(_buffers),
                                          absoluteStartIndex(_begin), absoluteEndIndex(_end),
                                          absoluteCurrentIndex(_begin), currentBuffer(0), currentIndex(0) {
    adjustIndexes();
}

void GIstreambuff::setup(std::size_t _start, std::size_t _end) {
    absoluteStartIndex = _start;
    absoluteEndIndex = _end,
    absoluteCurrentIndex = _start;
    adjustIndexes();
}

void GIstreambuff::adjustIndexes() {
    currentBuffer = 0;
    currentIndex = 0;
    //calculate the buffer and index
    std::size_t charCount = 0;
    //well, we could substract from absolute index, but ...
    for(unsigned int i = 0; i < buffers.size(); ++i) {
        std::size_t len = buffers.at(i).get().marker();
        std::size_t nextM = charCount + len;
        if(charCount <= absoluteCurrentIndex && absoluteCurrentIndex < nextM) {
            currentIndex = absoluteCurrentIndex - charCount;
            break;
        }
        currentBuffer++;
        charCount = nextM;
    }
    if(currentBuffer == buffers.size() && currentBuffer != 0) {
        currentBuffer = buffers.size() - 1;
        currentIndex = buffers.at(currentBuffer).get().marker(); //end
    }
}


GIstreambuff::int_type GIstreambuff::underflow() {
    if (absoluteCurrentIndex == absoluteEndIndex) {
        return traits_type::eof();
    }

    GBuffer b = buffers.at(currentBuffer).get();

    return traits_type::to_int_type(b.buffer()[currentIndex]);
}


GIstreambuff::int_type GIstreambuff::uflow() {
    if (absoluteCurrentIndex == absoluteEndIndex) {
        return traits_type::eof();
    }
    GBuffer b = buffers.at(currentBuffer).get();
    int_type ret = traits_type::to_int_type(b.buffer()[currentIndex]);
    //now increment me
    ++absoluteCurrentIndex;
    ++currentIndex;
    if(currentIndex == b.marker() && currentBuffer < buffers.size() - 1) {
        ++currentBuffer;
        currentIndex = 0;
    }
    return ret;
}


GIstreambuff::int_type GIstreambuff::pbackfail(int_type ch) {
    if (absoluteCurrentIndex == absoluteStartIndex) {
        return traits_type::eof();
    }
    //now decrement me
    GBuffer b = buffers.at(currentBuffer).get();
    --absoluteCurrentIndex;
    if(currentIndex > 0) {
        --currentIndex;
    } else if(currentBuffer > 0){
        --currentBuffer;
        b = buffers.at(currentBuffer).get();
        currentIndex = b.marker() - 1;
    }
    return traits_type::to_int_type(b.buffer()[currentIndex]);
}


std::streamsize GIstreambuff::showmanyc() {
    return absoluteEndIndex - absoluteCurrentIndex;
}


} }
