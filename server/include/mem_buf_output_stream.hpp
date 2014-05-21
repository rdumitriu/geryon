///
/// \file mem_buf_output_stream.hpp
///
///  Created on: May 20, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef MEMBUFOUTPUTSTREAM_HPP_
#define MEMBUFOUTPUTSTREAM_HPP_

#include <streambuf>

#include "mem_buf.hpp"

namespace geryon { namespace server {

class GOstreambuff : public std::streambuf {
public:
    GOstreambuff(GMemoryPool *_pGPool);
    virtual ~GOstreambuff();

    ///Non-Copyable
    GOstreambuff(const GOstreambuff & other) = delete;
    ///Non-Copyable
    GOstreambuff & operator = (const GOstreambuff & other) = delete;
protected:
    int_type overflow(int_type ch);
    int sync();

private:
    GMemoryPool * pGPool;
    GBufferHandler buf_handler;
};

} } /* namespace */

#endif
