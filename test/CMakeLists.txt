cmake_minimum_required(VERSION 3.0)

project(test_nbiot_sdk CXX)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

aux_source_directory(. TEST_SOURCES)

add_executable(
    ${PROJECT_NAME}
    ${TEST_SOURCES}
)

if(WIN32)
target_link_libraries(
    ${PROJECT_NAME}
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    nbiot_sdk
    ws2_32
)
else(WIN32)
target_link_libraries(
    ${PROJECT_NAME}
    ${GTEST_LIBRARY}
    ${GTEST_MAIN_LIBRARY}
    nbiot_sdk
    pthread
)
endif(WIN32)