cmake_minimum_required(VERSION 3.10)
project(deps)

add_subdirectory(cista EXCLUDE_FROM_ALL)
add_subdirectory(deboost.context EXCLUDE_FROM_ALL)
add_subdirectory(ctx EXCLUDE_FROM_ALL)
add_subdirectory(doctest EXCLUDE_FROM_ALL)
