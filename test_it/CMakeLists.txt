CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET (livetest_SRC
    test_filter_global.hpp
    test_filter_global.cpp

    test_filter_nrm.hpp
    test_filter_nrm.cpp

    test_servlet.hpp
    test_servlet.cpp

    tprogrammer.hpp
    tprogrammer_servlet.hpp
    tprogrammer_servlet.cpp

    livetest.cpp
	)

SET (MLIB geryon-commons ${LIBS})

ADD_LIBRARY(livetest SHARED ${livetest_SRC})

INCLUDE_DIRECTORIES ("../commons/include")

TARGET_LINK_LIBRARIES(livetest ${MLIB})

INSTALL(TARGETS livetest DESTINATION "${AGRADE_INSTALL_MODULES}")
