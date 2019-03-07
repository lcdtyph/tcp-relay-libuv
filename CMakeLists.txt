cmake_minimum_required(VERSION 3.0)
project(tcp-relay)

set(CMAKE_C_FLAGS_DEBUG "-g")

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads)

add_subdirectory(log.c)

list(APPEND SRCS
    src/buffer.c
    src/server.c
    src/tcp-relay.c
    )

list(APPEND DEPS uv log.c tbox Threads::Threads)

if (UNIX)
    list(APPEND DEPS m)
endif (UNIX)

include_directories(src/)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${DEPS})

