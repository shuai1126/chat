cmake_minimum_required(VERSION 3.15)
project(chat)

set(CMAKE_CXX_STANDARD 14)

add_executable(chat server.cpp client.cpp)