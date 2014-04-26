CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

enable_testing ()

SET (MLIB geryon-commons ${LIBS})

ADD_DEFINITIONS(-D_DEBUG)

# TESTS

ADD_EXECUTABLE(test_mem_buf_ump ../src/mem_buf.cpp test_mem_buf_ump.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_mem_buf_ump ${MLIB})

ADD_TEST(test_mem_buf_ump1 test_mem_buf_ump)
SET_TESTS_PROPERTIES (test_mem_buf_ump1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "ERROR.*")

