cmake_minimum_required(VERSION 3.18)
project(tg_bot_schedule)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(tg_bot_schedule
        sock.cpp
        sock.h)
