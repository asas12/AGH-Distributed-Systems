cmake_minimum_required(VERSION 3.13)
project(token_ring C)

set(CMAKE_C_STANDARD 11)


add_library(functions udp.h tcp.h)
target_sources(functions PUBLIC udp.c tcp.c)

add_executable(main main.c)

target_link_libraries(main functions)