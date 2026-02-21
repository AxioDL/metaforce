cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15...3.20)
project(bintoc LANGUAGES C)

add_executable(bintoc bintoc.c)

find_package(ZLIB REQUIRED)
target_link_libraries(bintoc PRIVATE ZLIB::ZLIB)

install(TARGETS bintoc DESTINATION bin)
