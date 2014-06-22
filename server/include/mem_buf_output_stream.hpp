///
/// \file mem_buf_output_stream.hpp
///
///  Created on: May 20, 2014
///      Author: rdumitriu at gmail.com
///

#ifndef GERYON_MEMBUFOUTPUTSTREAM_HPP_
#define GERYON_MEMBUFOUTPUTSTREAM_HPP_

#include <streambuf>

#include "mem_buf.hpp"
#include "tcp_protocol_handler.hpp"

namespace geryon { namespace server {

class GOstreambuff : public std::streambuf {
public:
    GOstreambuff(TCPProtocolHandler * pProtocolHandler);
    virtual ~GOstreambuff();

    ///Non-Copyable
    GOstreambuff(const GOstreambuff & other) = delete;
    ///Non-Copyable
    GOstreambuff & operator = (const GOstreambuff & other) = delete;

    void close();
protected:
    int_type overflow(int_type ch);
    int sync();

private:
    TCPProtocolHandler * pProtocolHandler;
    GBufferHandler buf_handler;
};

} } /* namespace */

#endif
