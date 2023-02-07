cmake_minimum_required(VERSION 3.6)

option(BUILD_EXAMPLE "build example" ON)
option(BUILD_SHARED "build shared library" ON)
option(BUILD_STATIC "build static library" ON)

project(tOO VERSION 1.0)

add_definitions("-Wall -g")

set(TOO_SRCS ./too_threadpool.c ./utils.c)
set(LIBS pthread)
if(BUILD_SHARED)
    add_library(too SHARED ${TOO_SRCS})
    target_include_directories(too PRIVATE ./)
    target_link_libraries(too ${LIBS})
endif()

if(BUILD_STATIC)
    add_library(too_static STATIC ${TOO_SRCS})
    target_include_directories(too_static PRIVATE ./)
    target_link_libraries(too_static ${LIBS})
endif()

if(BUILD_TEST)
	add_subdirectory(example)
endif()

