cmake_minimum_required(VERSION 3.0)

project(nbiot_sample C)

aux_source_directory(. NBIOT_SAMPLE_SOURCE)

add_executable(
    ${PROJECT_NAME}
    ${NBIOT_SAMPLE_SOURCE}
)

if(MSVC)
target_link_libraries(${PROJECT_NAME} nbiot_sdk ws2_32)
else()
target_link_libraries(${PROJECT_NAME} nbiot_sdk)
endif()