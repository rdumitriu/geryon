///
/// \file mem_buf_input_stream.hpp
///
///  Created on: May 20, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef MEMBUFINPUTSTREAM_HPP_
#define MEMBUFINPUTSTREAM_HPP_

#include <vector>
#include <algorithm>
#include <cstring>
#include <streambuf>

#include "mem_buf.hpp"

namespace geryon { namespace server {

class GIstreambuff : public std::streambuf {
public:
    explicit GIstreambuff(std::vector<GBufferHandler> & buffers, std::size_t begin = 0, std::size_t end = 0);
    virtual ~GIstreambuff() {}

    void setup(std::size_t start, std::size_t end);

    ///Non-Copyable
    GIstreambuff(const GIstreambuff & other) = delete;
    ///Non-Copyable
    GIstreambuff & operator = (const GIstreambuff & other) = delete;

protected:
    virtual int_type underflow();
    virtual int_type uflow();
    virtual int_type pbackfail(int_type ch);
    virtual std::streamsize showmanyc();
private:
    void adjustIndexes();
    std::vector<GBufferHandler> & buffers;
    std::size_t absoluteStartIndex;
    std::size_t absoluteEndIndex;

    std::size_t absoluteCurrentIndex; //the index, absolute
    std::size_t currentBuffer; //the current buffer
    std::size_t currentIndex; // the current index in the current buffer
};



} } /* namespace */

#endif
