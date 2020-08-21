project(aiso-coroutines-server)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(config)
add_subdirectory(util)

#add_executable(aiso-coroutines-server main.cpp app.cpp connection.cpp server.cpp)
#target_link_libraries(aiso-coroutines-server PUBLIC Boost::system beast_fun_times::util)

