cmake_minimum_required(VERSION 3.14)
project(archiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

add_executable(archiver src/main.cpp)

target_include_directories(archiver PRIVATE ./inc)