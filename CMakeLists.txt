cmake_minimum_required(VERSION 3.15)
project(crpc-Server)

set(CMAKE_CXX_STANDARD 20)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)

find_package(Boost COMPONENTS system thread REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(crpc-Server server.cpp)
target_link_libraries(crpc-Server ${USED_LIBS})

##################
project(crpc-Client)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost 1.72.0 REQUIRED system)
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
#add_subdirectory(cista)

add_executable(crpc-Client client.cpp)
target_link_libraries(crpc-Client ${USED_LIBS})
