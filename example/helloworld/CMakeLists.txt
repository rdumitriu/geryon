CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (helloworld_SRC
    json_utils.hpp
    helloresponse.hpp
    helloworld.cpp
	)

SET (MLIB geryon-commons ${LIBS})

ADD_LIBRARY(helloworldapp SHARED ${helloworld_SRC})

INCLUDE_DIRECTORIES ("../../commons/include")

TARGET_LINK_LIBRARIES(helloworldapp ${MLIB})

INSTALL(TARGETS helloworldapp DESTINATION "${AGRADE_INSTALL_MODULES}")
