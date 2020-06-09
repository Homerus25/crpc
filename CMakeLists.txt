cmake_minimum_required(VERSION 3.15)
project(crpc)

################################
# Dependencies
################################
find_package(Threads)

if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()
find_package(Boost 1.70.0 REQUIRED COMPONENTS system thread)
find_package(Threads)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS=1)  # fixes issue with Boost 1.72.0

include(cmake/pkg.cmake)


################################
# Linter
################################
option(CRPC_LINT "enable lint (clang-tidy) target" OFF)
if(CRPC_LINT)
  include(cmake/clang-tidy.cmake)
endif()


################################
# cprc Library
################################
file(GLOB_RECURSE crpc-src-files src/*.cc)
add_library(crpc ${crpc-src-files})
target_link_libraries(crpc cista ctx ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(crpc PUBLIC include)
target_compile_features(crpc PUBLIC cxx_std_20)


################################
# Tests
################################
file(GLOB_RECURSE crpc-src-test-files test/*.cc)
add_executable(crpc-test ${crpc-src-test-files})
target_link_libraries(crpc-test doctest crpc)


################################
# Example
################################
add_executable(crpc-example-server example/server.cc)
target_link_libraries(crpc-example-server crpc)

add_executable(crpc-example-client example/client.cc)
target_link_libraries(crpc-example-client crpc)

