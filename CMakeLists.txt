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
find_package(Threads)


link_directories(${Boost_LIBRARY_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR} deps/net/include)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "disable tests" FORCE)

include(cmake/pkg.cmake)
include_directories(deps/net)
include_directories(deps/benchmark/include)
include_directories(deps/mqtt_cpp/include)
include_directories(deps/glaze/include)
include_directories(external_lib)


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
set(crpc-src-files src/client.cc src/ws_client.cc src/server.cc)
set (THREADS_PREFER_PTHREAD_FLAG ON)
add_library(crpc ${crpc-src-files})
target_link_libraries(crpc cista ctx ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(crpc PUBLIC include)
target_compile_features(crpc PUBLIC cxx_std_20)
link_libraries(Boost::coroutine)


################################
# Tests
################################
find_package(doctest REQUIRED)
include_directories(${DOCTEST_INCLUDE_DIR})
add_executable(crpc-test test/main.cc test/no_network_test.cc)
target_include_directories(crpc-test PUBLIC ${DOCTEST_INCLUDE_DIR})
target_link_libraries(crpc-test doctest::doctest doctest crpc web-server http-client)


################################
# executables
################################

# http and websocket
add_executable(crpc-http-ws-server test/http_ws/http_ws_server.cc)
target_link_libraries(crpc-http-ws-server crpc web-server)

add_executable(crpc-http-client test/http_ws/rpc_http_client.cc)
target_link_libraries(crpc-http-client crpc http-client)

add_executable(crpc-ws-client test/http_ws/rpc_ws_client.cc src/ws_client.cc)
target_link_libraries(crpc-ws-client crpc)


# no network
add_executable(crpc-no-network-benchmark test/no_network/no_network.cc)
target_link_libraries(crpc-no-network-benchmark crpc)


# mqtt
find_package(Threads REQUIRED)
set(mqtt_cpp_iface_DIR mqtt_cpp)

add_executable(crpc-mqtt-client test/mqtt/mqtt-client.cc include/crpc/mqtt/rpc_mqtt_transport.h)
target_include_directories(crpc-mqtt-client PUBLIC deps/mqtt_cpp/include Boost)
target_link_libraries(crpc-mqtt-client crpc Boost::log)

add_executable(crpc-mqtt-server test/mqtt/mqtt-server.cc include/crpc/mqtt/rpc_mqtt_transport.h)
target_include_directories(crpc-mqtt-server PUBLIC deps/mqtt_cpp/include Boost)
target_link_libraries(crpc-mqtt-server crpc Boost::log)


################################
# benchmark
################################
add_executable(crpc-benchmark src/benchmark/benchmark.cc include/crpc/mqtt/rpc_mqtt_transport.h)
target_include_directories(crpc-benchmark PUBLIC deps/benchmark/include deps/mqtt_cpp/include)
target_link_libraries(crpc-benchmark crpc benchmark web-server http-client)