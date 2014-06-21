CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (commonslib_SRC
    #
    # Utilities
    #
    ../include/monitor_queue.hpp
    ../include/repetitive_runnable.hpp
    ../include/thread_pool.hpp

    ../include/string_utils.hpp
    string_utils.cpp

    # Not really needed, we may remove it
    ../include/properties_file.hpp
    properties_file.cpp

    ../include/log.hpp
    log.cpp

    ../include/platform.hpp

    #
    # JSON Support
    #
    ../include/json/json.hpp

    ../include/http_types.hpp
    http_types.cpp

    ../include/appexception.hpp

    ../include/application_config.hpp
    application_config.cpp

    ../include/appconfig_aware.hpp
    appconfig_aware.cpp

    ../include/session_listener.hpp

    ../include/session.hpp
    session.cpp

    ../include/filter.hpp

    ../include/servlet.hpp
    servlet.cpp

    ../include/application.hpp
    application.cpp

    ../include/sql/sql_pool.hpp

    ../include/sql/sql_postgres_support.hpp

    ../include/resources.hpp
)

INCLUDE_DIRECTORIES ("../include" )

ADD_LIBRARY(geryon-commons SHARED ${commonslib_SRC})

TARGET_LINK_LIBRARIES(geryon-commons ${LIBS})

INSTALL(TARGETS geryon-commons DESTINATION "${AGRADE_INSTALL_LIB}")
