cmake_minimum_required(VERSION 3.0)

project(nbiot_sdk)

option(UNIT_TEST "test nbiot sdk" 0)

if(WIN32)
    add_definitions(-DNBIOT_OS_WIN)
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    if(MSVC)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
        add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
        add_definitions(-Dsnprintf=_snprintf)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4819")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /DNBIOT_DEBUG /DLWM2M_WITH_LOGS")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4819")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /DNBIOT_DEBUG /DLWM2M_WITH_LOGS")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /HEAP:\"2048\" /STACK:\"1024\"")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /HEAP:\"2048\" /STACK:\"1024\"")
    endif(MSVC)
elseif(UNIX OR CYGWIN)
    add_definitions(-DNBIOT_OS_POSIX)

    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb3")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")

        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -ggdb3")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Os")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,.")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,.")
    endif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

    string(TOLOWER "${CMAKE_BUILD_TYPE}" __DEBUG)
    string(COMPARE EQUAL "${__DEBUG}" "debug" ___DEBUG)
    if(___DEBUG)
        add_definitions(-DNBIOT_DEBUG -DLWM2M_WITH_LOGS)
    endif(___DEBUG)
endif(WIN32)

include(${CMAKE_CURRENT_LIST_DIR}/wakaama/core/wakaama.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/platforms/platforms.cmake)

add_definitions(-DLWM2M_CLIENT_MODE)
#add_definitions(-DLWM2M_SUPPORT_JSON)
add_definitions(${WAKAAMA_DEFINITIONS})

include_directories(
    ${WAKAAMA_SOURCES_DIR}
    ${PLATFORMS_INCLUDE}
    ${CMAKE_CURRENT_LIST_DIR}/include
)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/source SOURCES)

add_library(
    ${PROJECT_NAME}
    ${SOURCES}
    ${WAKAAMA_SOURCES}
    ${PLATFORMS_SOURCE}
)

add_subdirectory(sample)

if(UNIT_TEST)
    add_subdirectory(test)
endif(UNIT_TEST)