cmake_minimum_required(VERSION 3.14)
project(archiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_SOURCE_DIR}/inc)
set(MIDDLEWARE_DIR ${CMAKE_SOURCE_DIR}/middlewares)

set(LIBARCHIVE_DIR ${MIDDLEWARE_DIR}/libarchive)
set(ARGPARSE_DIR ${MIDDLEWARE_DIR}/argparse)

add_subdirectory(${LIBARCHIVE_DIR})
add_subdirectory(${ARGPARSE_DIR})

add_executable(archiver ${SRC_DIR}/main.cpp ${SRC_DIR}/arguments.cpp ${SRC_DIR}/compression.cpp)

target_include_directories(archiver PRIVATE 
    ${INC_DIR}
    ${LIBARCHIVE_DIR}
    ${ARGPARSE_DIR}
 )

target_link_libraries(archiver PRIVATE 
    archive
    argparse
)