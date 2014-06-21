CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (livetest_SRC
    livetest.cpp
	)

SET (MLIB geryon-commons ${LIBS})

ADD_LIBRARY(livetest SHARED ${livetest_SRC})

INCLUDE_DIRECTORIES ("../../commons/include")

TARGET_LINK_LIBRARIES(livetest ${MLIB})

INSTALL(TARGETS livetest DESTINATION "${AGRADE_INSTALL_MODULES}")
