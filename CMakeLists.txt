cmake_minimum_required(VERSION 3.0)
project(tcp-relay)

set(CMAKE_C_FLAGS_DEBUG "-g")

add_definitions(-DLOG_USE_COLOR)
list(APPEND LOGC_SRCS log.c/src/log.c)

add_library(log.c STATIC ${LOGC_SRCS})
target_include_directories(log.c PUBLIC log.c/src)

list(APPEND SRCS
    src/buffer.c
    src/server.c
    src/tcp-relay.c
    )

list(APPEND DEPS uv log.c)

include_directories(src/)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${DEPS})

