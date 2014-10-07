
#include "mem_buf_input_stream.hpp"
#include "log.hpp"

namespace geryon { namespace server {


GIstreambuff::GIstreambuff(std::vector<GBufferHandler> & _buffers,
                           std::size_t _begin, std::size_t _end)
                                        : std::streambuf(), buffers(_buffers),
                                          absoluteStartIndex(_begin), absoluteEndIndex(_end),
                                          absoluteCurrentIndex(_begin), currentBuffer(0), currentIndex(0),
                                          currentGapIndex(-1), currentGap(0) {
    adjustIndexes();
}

void GIstreambuff::setup(std::size_t _start, std::size_t _end) {
    // assuming gaps are added after setup
    absoluteStartIndex = _start;
    absoluteEndIndex = _end,
    absoluteCurrentIndex = _start;
    currentGapIndex = -1;
    currentGap = 0;
    gaps.clear();
    adjustIndexes();
    LOG(geryon::util::Log::DEBUG) << "Input stream starts at:" << absoluteStartIndex
                                  << " and ends at:" << absoluteEndIndex;
}

void GIstreambuff::addGap(std::size_t start, std::size_t end) {
    detail::GISBuffGap gap;
    gap.start = start;
    gap.stop = end;
    gaps.push_back(std::move(gap));
    if(!currentGap) {
        currentGapIndex++;
        currentGap = &gaps.at(currentGapIndex);
    }
    if(absoluteCurrentIndex >= start && absoluteCurrentIndex < end) {
        advanceIndex(); // this will increment currentGapIndex just over the edge of gaps
        currentGapIndex--;
    }
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
    if (absoluteCurrentIndex >= absoluteEndIndex) {
        return traits_type::eof();
    }

    GBuffer b = buffers.at(currentBuffer).get();

    return traits_type::to_int_type(b.buffer()[currentIndex]);
}


GIstreambuff::int_type GIstreambuff::uflow() {
    int_type ret = underflow();
    advanceIndex();
    return ret;
}


GIstreambuff::int_type GIstreambuff::pbackfail(int_type ch) {
    rollbackIndex();
    if (absoluteCurrentIndex <= absoluteStartIndex) { 
        //absolute start may be in a gap
        return traits_type::eof();
    }
    GBuffer b = buffers.at(currentBuffer).get();
    return traits_type::to_int_type(b.buffer()[currentIndex]);
}


std::streamsize GIstreambuff::showmanyc() {
    return absoluteEndIndex - absoluteCurrentIndex; //::TODO:: care about the gaps starting from current index
}

void GIstreambuff::advanceIndex() {
    if(absoluteCurrentIndex >= absoluteEndIndex) {
        return;
    }
    GBuffer b;
    do {
        // move current gap pointer to next gap if we're at the end of the gap
        if(currentGap && absoluteCurrentIndex == currentGap->stop - 1) {
            ++currentGapIndex;
            if( currentGapIndex < gaps.size() ) { 
                currentGap = &gaps.at(currentGapIndex);
            } else {
                currentGap = 0;
            }
        }   
        // increment indexes
        ++absoluteCurrentIndex;
        ++currentIndex;
        b = buffers.at(currentBuffer).get();
        if(currentIndex == b.marker() && currentBuffer < buffers.size() - 1) {
            ++currentBuffer;
            currentIndex = 0;
        }
    // repeat while we're inside a gap
    // unless there's a gap after the absoluteEndIndex, this will not move the index after eof
    } while(currentGap && absoluteCurrentIndex >= currentGap->start);
}

void GIstreambuff::rollbackIndex() {
    if(absoluteCurrentIndex <= absoluteStartIndex) {
        return;
    }
    do {
        // move current gap pointer to previous gap if we're at the start of the gap
        if(currentGap && absoluteCurrentIndex == currentGap->start) {
            --currentGapIndex;
            if(currentGapIndex >= 0) {
                currentGap = &gaps.at(currentGapIndex);
            } else {
                currentGap = 0;
            }
        }   
        // increment indexes
        --absoluteCurrentIndex;
        if(currentIndex > 0) {
            --currentIndex;
        } else if(currentBuffer > 0) {
            --currentBuffer;
            GBuffer b = buffers.at(currentBuffer).get();
            currentIndex = b.marker() - 1;
        }
    // repeat while we're inside a gap
    // unless there's a gap before the absoluteStartIndex, this will not move the index before the start
    } while(currentGap && absoluteCurrentIndex < currentGap->stop);
}


} }
