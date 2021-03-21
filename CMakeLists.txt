# Prerequisites.
cmake_minimum_required(VERSION 3.16)
set (CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
include(GNUInstallDirs)

# Project config.
project(lingo)

# Tests.
find_package(Catch2 REQUIRED)
add_executable(parse_tests test/parse-tests.cpp)
target_link_libraries(parse_tests PRIVATE Catch2::Catch2)
target_include_directories(parse_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
include(CTest)
include(Catch)
catch_discover_tests(parse_tests)
enable_testing()