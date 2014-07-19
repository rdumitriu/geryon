CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

enable_testing ()

SET (MLIB geryon-commons ${LIBS})

ADD_DEFINITIONS(-D_DEBUG)

# LOG TESTS

ADD_EXECUTABLE(testlog test_log.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (testlog  ${MLIB})

ADD_EXECUTABLE(testconv test_conversions.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (testconv ${MLIB})

ADD_EXECUTABLE(testprops test_properties.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (testprops  ${MLIB})

ADD_EXECUTABLE(test_monitor_queue test_monitor_queue.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_monitor_queue  ${MLIB})

ADD_EXECUTABLE(test_rrunnable test_repetitive_runnable.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_rrunnable  ${MLIB})

ADD_EXECUTABLE(test_thrpool test_thread_pool.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_thrpool  ${MLIB})

ADD_EXECUTABLE(test_http_message test_http_message.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_http_message  ${MLIB})

ADD_EXECUTABLE(test_session test_session.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_session  ${MLIB})

ADD_EXECUTABLE(test_app test_app.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_app  ${MLIB})

ADD_EXECUTABLE(test_sql_pool test_sql_pool.cpp)
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_sql_pool  ${MLIB})

ADD_TEST(testlog1 testlog)
SET_TESTS_PROPERTIES (testlog1
                      PROPERTIES
                      PASS_REGULAR_EXPRESSION "Log works.*")
ADD_TEST(testlog2 testlog)
SET_TESTS_PROPERTIES (testlog2
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED:.*")

ADD_TEST(testconv1 testconv)
SET_TESTS_PROPERTIES (testconv1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED:.*")

ADD_TEST(testprops1 testprops ${CMAKE_CURRENT_SOURCE_DIR}/props.properties)
SET_TESTS_PROPERTIES (testprops1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED")

ADD_TEST(test_monitor_queue1 test_monitor_queue)
SET_TESTS_PROPERTIES (test_monitor_queue1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED:.*")

ADD_TEST(test_rrunnable1 test_rrunnable)
SET_TESTS_PROPERTIES (test_rrunnable1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")

ADD_TEST(test_thrpool1 test_thrpool)
SET_TESTS_PROPERTIES (test_thrpool1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")

ADD_TEST(test_http_message1 test_http_message)
SET_TESTS_PROPERTIES (test_http_message1
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")

ADD_TEST(test_session test_session)
SET_TESTS_PROPERTIES (test_session
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")

ADD_TEST(test_app test_app)
SET_TESTS_PROPERTIES (test_app
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")

ADD_TEST(test_sql_pool test_sql_pool)
SET_TESTS_PROPERTIES (test_sql_pool
                      PROPERTIES
                      FAIL_REGULAR_EXPRESSION "FAILED.*")
