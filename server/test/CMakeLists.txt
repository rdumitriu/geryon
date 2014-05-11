CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

enable_testing ()

SET (MLIB geryon-commons ${LIBS})

ADD_DEFINITIONS(-D_DEBUG)

# TESTS

ADD_EXECUTABLE(test_mem_buf_ump ../src/mem_buf.cpp test_mem_buf_ump.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_mem_buf_ump ${MLIB})

ADD_EXECUTABLE(test_mem_buf_hnd ../src/mem_buf.cpp test_mem_buf_hnd.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_mem_buf_hnd ${MLIB})

ADD_EXECUTABLE(test_mem_buf_prf ../src/mem_buf.cpp test_mem_buf_prf.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_mem_buf_prf ${MLIB})

ADD_TEST(test_mem_buf_ump1 test_mem_buf_ump)
SET_TESTS_PROPERTIES (test_mem_buf_ump1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "ERROR.*")

ADD_TEST(test_mem_buf_ump2 test_mem_buf_ump 2)
SET_TESTS_PROPERTIES (test_mem_buf_ump2
                      PROPERTIES
                      PASS_REGULAR_EXPRESSION ".*we failed correctly.*")

ADD_TEST(test_mem_buf_hnd1 test_mem_buf_hnd)
SET_TESTS_PROPERTIES (test_mem_buf_hnd1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "ERROR.*")
