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

ADD_EXECUTABLE(test_buf_istream ../src/mem_buf.cpp ../src/mem_buf_input_stream.cpp test_buf_istream.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_buf_istream ${MLIB})

ADD_EXECUTABLE(test_buf_ostream ../src/mem_buf.cpp ../src/mem_buf_output_stream.cpp
               ../src/tcp_connection.cpp ../src/tcp_connection_manager.cpp ../src/tcp_protocol_handler.cpp test_buf_ostream.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_buf_ostream ${MLIB})

ADD_EXECUTABLE(test_path_tree test_path_tree.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_path_tree ${MLIB})

ADD_EXECUTABLE(test_matching_entry test_matching_entry.cpp ../src/server_application_utils.cpp)
INCLUDE_DIRECTORIES ("../include" "../../commons/include")
TARGET_LINK_LIBRARIES (test_matching_entry ${MLIB})

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

ADD_TEST(test_mem_buf_prf1 test_mem_buf_prf)
SET_TESTS_PROPERTIES (test_mem_buf_prf1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "ERROR.*")

ADD_TEST(test_buf_istream1 test_buf_istream)
SET_TESTS_PROPERTIES (test_buf_istream1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "ERROR.*")

ADD_TEST(test_buf_ostream1 test_buf_ostream)
SET_TESTS_PROPERTIES (test_buf_ostream1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "ERROR.*")

ADD_TEST(test_path_tree1 test_path_tree)
SET_TESTS_PROPERTIES (test_path_tree1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")
