# Prerequisites.
cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
include(GNUInstallDirs)

# Project config.
project(lingo)

# Tests.
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})
add_executable(tests 
    test/parse-tests.cpp 
    test/ast-tests.cpp)
target_link_libraries(tests PRIVATE GTest::GTest GTest::Main)