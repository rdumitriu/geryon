
#include "mem_buf_output_stream.hpp"

namespace geryon { namespace server {

GOstreambuff::GOstreambuff(GMemoryPool *_pGPool) : std::streambuf(), pGPool(_pGPool), buf_handler(_pGPool, 0, false) {
}

GOstreambuff::~GOstreambuff() {
}

GOstreambuff::int_type GOstreambuff::overflow(GOstreambuff::int_type ch) {
    if(ch != traits_type::eof()) {
        GBuffer & gb = buf_handler.get(); //obviously, we need reference here!
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
//    std::cout << "Flushing buffer, marker is=" << buf_handler.get().marker() << ":[[";
//    for(unsigned int i = 0; i < buf_handler.get().marker(); ++i) {
//        std::cout << buf_handler.get().buffer()[i];
//    }
//    std::cout << "]]" << std::endl;

    GBufferHandler temp(pGPool);
    buf_handler = std::move(temp);
    return 0;
}


} }
