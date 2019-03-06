cmake_minimum_required(VERSION 3.0)
project(tcp-relay)

set(CMAKE_C_FLAGS_DEBUG "-g")

add_subdirectory(log.c)

list(APPEND SRCS
    src/buffer.c
    src/server.c
    src/tcp-relay.c
    )

list(APPEND DEPS uv log.c tbox)

include_directories(src/)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${DEPS})

