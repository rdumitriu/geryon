
PROJECT (yahsrv)

set (GERYON_VERSION_MAJOR ${GERYON_SERVER_BUILD_VERSION_MAJOR})
set (GERYON_VERSION_MINOR ${GERYON_SERVER_BUILD_VERSION_MINOR})
set (GERYON_VERSION_BUILDNO ${GERYON_SERVER_BUILD_IDENTIFICATION})

CONFIGURE_FILE (
  "${PROJECT_SOURCE_DIR}/../include/server_build.hpp.in"
  "${PROJECT_BINARY_DIR}/../include/server_build.hpp"
)

SET (geryon_SRC
    ../include/os_utils.hpp
    os_utils.cpp

#
# MEMORY BUFFERS
#
    ../include/mem_buf.hpp
    mem_buf.cpp

    ../include/mem_buf_input_stream.hpp
    mem_buf_input_stream.cpp

    ../include/mem_buf_output_stream.hpp
    mem_buf_output_stream.cpp

#
# GLOBALS
#

    ../include/postgres_support.hpp
    postgres_support.cpp

    ../include/server_global_structs.hpp
    server_global_structs.cpp

#
# TCP
#
    ../include/tcp_protocol.hpp

    ../include/tcp_connection.hpp
    tcp_connection.cpp

    ../include/tcp_connection_manager.hpp
    tcp_connection_manager.cpp

    ../include/tcp_protocol_handler.hpp
    tcp_protocol_handler.cpp

    ../include/tcp_server.hpp
    tcp_server.cpp

    ../include/tcp_sta_server.hpp
    tcp_sta_server.cpp

    ../include/tcp_mta_server.hpp
    tcp_mta_server.cpp

#
# GADM
#
    ../include/gadm_protocol.hpp
    gadm_protocol.cpp

#
# SERVER APPLICATION
#
    ../include/server_session.hpp

    ../include/server_application_utils.hpp
    server_application_utils.cpp

    ../include/server_application_fltch.hpp
    server_application_fltch.cpp

    ../include/server_application_srvdsp.hpp
    server_application_srvdsp.cpp

    ../include/server_application.hpp
    server_application.cpp

#
# PROTOCOL : HTTP
#

    ../include/http_executor.hpp
    http_executor.cpp

    ../include/http_st_executor.hpp
    http_st_executor.cpp

    ../include/http_mt_executor.hpp
    http_mt_executor.cpp

    ../include/http_protocol.hpp
    http_protocol.cpp

    ../include/http_protocol_handler.hpp
    http_protocol_handler.cpp

    ../include/http_server_types.hpp
    http_server_types.cpp

    ../include/http_request_parser.hpp
    http_request_parser.cpp
#
# MAIN
#

    ../include/geryon_configsql_postgres.hpp
    geryon_configsql_postgres.cpp

    ../include/geryon_configapp.hpp
    geryon_configapp.cpp

    ../include/geryon_config.hpp
    geryon_config.cpp

    main.cpp
     )

ADD_EXECUTABLE(geryon ${geryon_SRC})

INCLUDE_DIRECTORIES ("../include" "../../commons/include" "${PROJECT_BINARY_DIR}/../include/")

TARGET_LINK_LIBRARIES(geryon ${LIBS} ${AGRADE_LIBS})

INSTALL(TARGETS geryon DESTINATION "${AGRADE_INSTALL_BIN}")
