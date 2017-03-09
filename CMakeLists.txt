cmake_minimum_required(VERSION 3.0)

project(nbiot_sdk)

option(UNIT_TEST  "unit test" 0)
option(HAVE_DTLS  "use dtls" 0)
option(BIG_ENDIAN "big endian" 0)
option(WITH_LOGS  "print logs" 0)
option(BOOTSTRAP  "support boostrap" 0)

if(WIN32)
    set(NBIOT_WIN 1)
    add_definitions(-DNBIOT_WIN)
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    if(MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
        add_definitions(-Dinline=__inline)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4819")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /DNBIOT_DEBUG")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /DNBIOT_DEBUG")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /HEAP:\"2048\" /STACK:\"1024\"")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /HEAP:\"2048\" /STACK:\"1024\"")
    endif()
elseif(UNIX OR CYGWIN)
    set(NBIOT_POSIX 1)
    add_definitions(-DNBIOT_POSIX)
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-DNBIOT_DEBUG)
    endif()

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -ggdb3")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Os")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,.")
    endif()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/platforms/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/source/dtls/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/source/lwm2m/CMakeLists.txt)

add_definitions(
    ${PLATFORMS_DEFINITIONS}
    ${LWM2M_DEFINITIONS}
    ${DTLS_DEFINITIONS}
)

include_directories(
    ${PLATFORMS_INCLUDE}
    ${LWM2M_INCLUDE}
    ${DTLS_INCLUDE}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/source SOURCE)

add_library(
    ${PROJECT_NAME}
    ${SOURCE}
    ${DTLS_SOURCE}
    ${LWM2M_SOURCE}
    ${PLATFORMS_SOURCE}
)

add_subdirectory(sample)

if(UNIT_TEST)
    add_subdirectory(test)
endif(UNIT_TEST)