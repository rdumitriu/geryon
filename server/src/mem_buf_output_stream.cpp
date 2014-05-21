
#include "mem_buf_output_stream.hpp"

namespace geryon { namespace server {

GOstreambuff::GOstreambuff(TCPProtocolHandler * _pProtocolHandler)
                                : std::streambuf(),
                                  pProtocolHandler(_pProtocolHandler),
                                  buf_handler(_pProtocolHandler->getMemoryPool(), 0, false) {
}

GOstreambuff::~GOstreambuff() {
}

GOstreambuff::int_type GOstreambuff::overflow(GOstreambuff::int_type ch) {
    if(ch != traits_type::eof()) {
        //obviously, we need reference here! (11 minutes of anger!)
        GBuffer & gb = buf_handler.get();
        gb.buffer()[gb.marker()] = ch;
        gb.advanceMarker(1);
        if(gb.marker() == gb.size()) {
            sync();
        }
    } else {
        sync();
    }
    return ch;
}

int GOstreambuff::sync() {
    pProtocolHandler->requestWrite(std::move(buf_handler));
    GBufferHandler temp(pProtocolHandler->getMemoryPool());
    buf_handler = std::move(temp);
    return 0;
}


} }
