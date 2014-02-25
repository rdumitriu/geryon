CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

enable_testing ()

SET (MLIB geryon-commons ${LIBS})

ADD_DEFINITIONS(-D_DEBUG)

# LOG TESTS
#
SET (testlog_SRC
    test_log.cpp)

SET (testconversions_SRC
    test_conversions.cpp)

SET (test_monitor_queue_SRC
    test_monitor_queue.cpp)

SET (test_rrunnable_SRC
    test_repetitive_runnable.cpp)

SET (test_thrpool_SRC
    test_thread_pool.cpp)


ADD_EXECUTABLE(testlog ${testlog_SRC})
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (testlog  ${MLIB})

ADD_EXECUTABLE(testconv ${testconversions_SRC})
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (testconv  ${MLIB})

ADD_EXECUTABLE(test_monitor_queue ${test_monitor_queue_SRC})
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_monitor_queue  ${MLIB})

ADD_EXECUTABLE(test_rrunnable ${test_rrunnable_SRC})
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_rrunnable  ${MLIB})

ADD_EXECUTABLE(test_thrpool ${test_thrpool_SRC})
INCLUDE_DIRECTORIES ("../include")
TARGET_LINK_LIBRARIES (test_thrpool  ${MLIB})


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
