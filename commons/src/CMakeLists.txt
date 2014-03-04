CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (commonslib_SRC
    ../include/monitor_queue.hpp
    ../include/repetitive_runnable.hpp
    ../include/thread_pool.hpp

    ../include/string_utils.hpp
    ../include/log.hpp
    log.cpp

    ../include/http_types.hpp
    http_types.cpp

    ../include/appconfig_aware.hpp
    ../include/session_listener.hpp
    ../include/session.hpp
    ../include/filter.hpp
    ../include/servlet.hpp
    servlet.cpp

    ../include/application.hpp
    application.cpp
)

INCLUDE_DIRECTORIES ("../include" )

ADD_LIBRARY(geryon-commons STATIC ${commonslib_SRC})

TARGET_LINK_LIBRARIES(geryon-commons ${LIBS})

INSTALL(TARGETS geryon-commons DESTINATION "${AGRADE_INSTALL_LIB}")
