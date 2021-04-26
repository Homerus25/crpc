cmake_minimum_required(VERSION 3.15)
project(crpc)

set(CMAKE_CXX_STANDARD 20)

################################
# Dependencies
################################
find_package(Threads)

if (MSVC)
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
endif()

find_package(Boost REQUIRED COMPONENTS system thread coroutine log filesystem REQUIRED)
#add_link_options(crpc-client-mqtt "-lboost_log -lboost_filesystem -lboost_thread")

#find_package(Boost REQUIRED COMPONENTS system thread coroutine REQUIRED)
find_package(Threads)


link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR} deps/net/include)
#add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS=1)  # fixes issue with Boost 1.72.0

#include(cmake/pkg.cmake)
add_subdirectory(deps)


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
set (THREADS_PREFER_PTHREAD_FLAG ON)
add_library(crpc ${crpc-src-files})
target_link_libraries(crpc cista ctx ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(crpc PUBLIC include)
target_compile_features(crpc PUBLIC cxx_std_20)
link_libraries(Boost::coroutine)


################################
# Tests
################################
file(GLOB_RECURSE crpc-src-test-files test/*.cc)
add_executable(crpc-test ${crpc-src-test-files})
target_link_libraries(crpc-test doctest crpc)


################################
# Example
################################
#add_definitions(__clang__)

add_executable(crpc-example-server example/server.cc)
target_link_libraries(crpc-example-server crpc)

add_executable(crpc-example-client example/client.cc)
target_link_libraries(crpc-example-client crpc)

add_executable(crpc-example-client-async example/async_client.cc)
target_link_libraries(crpc-example-client-async crpc)

#add_executable(crpc-example-client-async-thread-websocket example/websocket_async_thread_client.cc)
#target_link_libraries(crpc-example-client-async-thread-websocket crpc)

#add_executable(crpc-example-server-async-thread-websocket example/websocket_async_thread_server.cc)
#target_link_libraries(crpc-example-server-async-thread-websocket crpc)

add_executable(crpc-example-server-async-websocket-net example/websocket_async_net_server.cc)
target_link_libraries(crpc-example-server-async-websocket-net crpc web-server)
#target_compile_features(crpc-example-server-async-websocket INTERFACE cxx_std_17)

add_executable(crpc-example-client-async-websocket-net example/websocket_async_net_client.cc src/ws_client.cpp)
target_link_libraries(crpc-example-client-async-websocket-net crpc)


add_executable(crpc-client-benchmark example/client_benchmarks.cc src/ws_client.cpp)
target_link_libraries(crpc-client-benchmark crpc)

#######

#add_subdirectory(include/crpc/mqtt_cpp)

#add_compile_definitions(crpc-client-mqtt "-DMQTT_USE_LOG -DBOOST_LOG_DYN_LINK -lboost_log -lboost_filesystem -lboost_thread")
#add_compile_definitions(crpc-client-mqtt "-lboost_log -lboost_filesystem -lboost_thread")
#add_link_options(crpc-client-mqtt "-lboost_log -lboost_filesystem -lboost_thread")

set(mqtt_cpp_iface_DIR mqtt_cpp)
add_executable(crpc-client-mqtt example/mqtt-client.cpp include/crpc/rpc_mqtt_transport.h)
target_include_directories(crpc-client-mqtt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/crpc/mqtt_cpp/include Boost )
target_link_libraries(crpc-client-mqtt crpc Boost::log)

add_executable(crpc-server-mqtt example/mqtt-server.cpp include/crpc/rpc_mqtt_transport.h)
target_include_directories(crpc-server-mqtt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/crpc/mqtt_cpp/include Boost )
target_link_libraries(crpc-server-mqtt crpc Boost::log)

###############################
if(FALSE)
add_subdirectory(coroutines)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS=1 -BOOST_ASIO_NO_DEPRECATED=1)  # fixes issue with Boost 1.72.0

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-fcoroutines)

add_executable(aiso-coroutines-server coroutines/main.cpp coroutines/app.cpp coroutines/connection.cpp coroutines/server.cpp)
target_link_libraries(aiso-coroutines-server PUBLIC Boost::system crpc beast_fun_times::util -fcoroutines)

target_link_options(aiso-coroutines-server PUBLIC -fcoroutines)
ENDIF()
