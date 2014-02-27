CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (commonslib_SRC
     ../include/monitor_queue.hpp
     ../include/repetitive_runnable.hpp
     ../include/thread_pool.hpp

     ../include/http_types.hpp
     http_types.cpp

     ../include/conversions.hpp
     ../include/log.hpp
     log.cpp
	)

INCLUDE_DIRECTORIES ("../include" )

ADD_LIBRARY(geryon-commons STATIC ${commonslib_SRC})

TARGET_LINK_LIBRARIES(geryon-commons ${LIBS})

INSTALL(TARGETS geryon-commons DESTINATION "${AGRADE_INSTALL_LIB}")
