
PROJECT (yahsrv)

set (GERYON_VERSION_MAJOR 0)
set (GERYON_VERSION_MINOR 1)
set (GERYON_VERSION_BUILDNO 201402-1)

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/../include/server_build.hpp.in"
  "${PROJECT_BINARY_DIR}/../include/server_build.hpp"
)

SET (geryon_SRC
    ../include/os_utils.hpp
    os_utils.cpp

    ../include/mem_buf.hpp
    mem_buf.cpp
    ../include/mem_buf_input_stream.hpp
    mem_buf_input_stream.cpp
    ../include/mem_buf_output_stream.hpp
    mem_buf_output_stream.cpp

    ../include/server_global_structs.hpp
    server_global_structs.cpp

    ../include/tcp_protocol.hpp

    ../include/tcp_connection.hpp
    tcp_connection.cpp
    ../include/tcp_connection_manager.hpp
    tcp_connection_manager.cpp
    ../include/tcp_protocol_handler.hpp
    tcp_protocol_handler.cpp
    ../include/tcp_server.hpp
    tcp_server.cpp
    ../include/tcp_st_server.hpp
    tcp_st_server.cpp
    ../include/tcp_mt_server.hpp
    tcp_mt_server.cpp

    ../include/gadm_protocol.hpp
    gadm_protocol.cpp

    ../include/http_protocol.hpp
    http_protocol.cpp

    ../include/http_protocol_handler.hpp
    http_protocol_handler.cpp

    ../include/http_server_types.hpp
    http_server_types.cpp

    ../include/http_request_parser.hpp
    http_request_parser.cpp

    main.cpp
     )

ADD_EXECUTABLE(geryon ${geryon_SRC})

INCLUDE_DIRECTORIES ("../include" "../../commons/include" "${PROJECT_BINARY_DIR}/../include/")

TARGET_LINK_LIBRARIES(geryon ${LIBS} ${AGRADE_LIBS})

INSTALL(TARGETS geryon DESTINATION "${AGRADE_INSTALL_BIN}")
