cmake_minimum_required(VERSION 3.16)
project(neptuneorm)

set(CMAKE_CXX_STANDARD 11)

set(MARIADB_LIBRARY_DIR "C:/Program Files/MariaDB/MariaDB C++ Connector 64-bit")

file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

include_directories(${MARIADB_LIBRARY_DIR}/include)
link_directories(${MARIADB_LIBRARY_DIR})

add_library(neptuneorm ${SRC_FILES})

target_link_libraries(neptuneorm PRIVATE mariadbcpp)
target_include_directories(neptuneorm PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(dev)