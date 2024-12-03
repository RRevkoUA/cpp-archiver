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

enable_testing()

#prepare test files
add_custom_target(PrepareTestFiles
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/test
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/test_files/ ${CMAKE_BINARY_DIR}/test
)

add_dependencies(archiver PrepareTestFiles)


# success tests
add_test(NAME test_help COMMAND archiver -h)
add_test(NAME test_version COMMAND archiver -v)

##compress
#compress all files in test directory
add_test(NAME s_test_compress COMMAND archiver -c ${CMAKE_BINARY_DIR}/test) 
add_test(NAME s_test_compress_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test.tar.gz
)
set_tests_properties(s_test_compress_check PROPERTIES 
    DEPENDS s_test_compress
)

#compress single file
add_test(NAME s_test_compress_single_file COMMAND archiver -c ${CMAKE_BINARY_DIR}/test/testfile.txt) #compress single file
add_test(NAME s_test_compress_single_file_check 
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/testfile.txt.tar.gz
)
set_tests_properties(s_test_compress_single_file_check PROPERTIES 
    DEPENDS s_test_compress_single_file
)

add_test(NAME s_test_compress_single_file_to_dir COMMAND archiver -c ${CMAKE_BINARY_DIR}/test/testfile.txt ${CMAKE_BINARY_DIR}/test) #compress single file to dir
add_test(NAME s_test_compress_single_file_to_dir_check 
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test/testfile.txt.tar.gz
)
set_tests_properties(s_test_compress_single_file_to_dir_check PROPERTIES 
    DEPENDS s_test_compress_single_file_to_dir
)

#compress all files in test_files directory with zip
add_test(NAME s_test_compress_type COMMAND archiver -c ${CMAKE_BINARY_DIR}/test -t zip) 
add_test(NAME s_test_compress_type_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test.tar.zip
)
set_tests_properties(s_test_compress_type_check PROPERTIES 
    DEPENDS s_test_compress_type
)

#compress all files in test_files directory to directory
add_test(NAME s_test_compress_type_without_key COMMAND archiver -c ${CMAKE_BINARY_DIR}/test ${CMAKE_BINARY_DIR}/test/test_files.tar.zip) 
add_test(NAME s_test_compress_type_without_key_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test/test_files.tar.zip
)
set_tests_properties(s_test_compress_type_without_key_check PROPERTIES 
    DEPENDS s_test_compress_type_without_key
)

#compress all files in test_files directory to directory with custom name
add_test(NAME s_test_compress_custom_name COMMAND archiver -c ${CMAKE_BINARY_DIR}/test ${CMAKE_BINARY_DIR}/test/custom.tar.gz) 
add_test(NAME s_test_compress_custom_name_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test/custom.tar.gz
)
set_tests_properties(s_test_compress_custom_name_check PROPERTIES 
    DEPENDS s_test_compress_custom_name
)

##extract
#extract single file
add_test(NAME s_test_extract COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/weight.tar.gz)
add_test(NAME s_test_extract_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/weight.txt
)
set_tests_properties(s_test_extract_check PROPERTIES 
    DEPENDS s_test_extract
)

#extract single file to directory
add_test(NAME s_test_extract_to_dir COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/weight.tar.gz ${CMAKE_BINARY_DIR}/test/)
add_test(NAME s_test_extract_to_dir_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test/weight.txt
)
set_tests_properties(s_test_extract_to_dir_check PROPERTIES 
    DEPENDS s_test_extract_to_dir
)

#extract single file with in-name type
add_test(NAME s_test_extract_type_without_key COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/sample_archive.tar.7z)
add_test(NAME s_test_extract_type_without_key_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/images.png
)

#extract single file with 7z to folder
add_test(NAME s_test_extract_type COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/sample_archive.tar.7z ${CMAKE_BINARY_DIR}/test -t 7z) 
add_test(NAME s_test_extract_type_check
    COMMAND ${CMAKE_COMMAND} -E env stat ${CMAKE_BINARY_DIR}/test/images.png
)
set_tests_properties(s_test_extract_type_check PROPERTIES 
    DEPENDS s_test_extract_type
)


# fail tests
add_test(NAME f_test_compress_and_extract COMMAND archiver -c . -e ${CMAKE_BINARY_DIR}/test/weight.tar.gz) #compress single file
set_tests_properties(f_test_compress_and_extract PROPERTIES WILL_FAIL TRUE)

##compress
add_test(NAME f_test_compress_invalid_dir COMMAND archiver -c ${CMAKE_BINARY_DIR}/test/invalid_dir) #compress single file
set_tests_properties(f_test_compress_invalid_dir PROPERTIES WILL_FAIL TRUE)

add_test(NAME f_test_compress_invalid_file COMMAND archiver -c ${CMAKE_BINARY_DIR}/test/invalid_file.txt) #compress single file
set_tests_properties(f_test_compress_invalid_file PROPERTIES WILL_FAIL TRUE)

##extract
add_test(NAME f_test_extract_invalid_file COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/invalid_file.txt) #extract single file
set_tests_properties(f_test_extract_invalid_file PROPERTIES WILL_FAIL TRUE)

add_test(NAME f_test_extract_invalid_dir COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/weight.tar.gz ${CMAKE_BINARY_DIR}/test/invalid_dir) #extract single file
set_tests_properties(f_test_extract_invalid_dir PROPERTIES WILL_FAIL TRUE)

add_test(NAME f_test_extract_invalid_type COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/sample_archive.tar.7z ${CMAKE_BINARY_DIR}/test -t invalid_type) #extract single file
set_tests_properties(f_test_extract_invalid_type PROPERTIES WILL_FAIL TRUE)

add_test(NAME f_test_extract_invalid_archive COMMAND archiver -e ${CMAKE_BINARY_DIR}/test/invalid_archive.tar.gz) #extract single file
set_tests_properties(f_test_extract_invalid_archive PROPERTIES WILL_FAIL TRUE)