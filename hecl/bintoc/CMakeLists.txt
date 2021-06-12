cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15...3.20)
project(bintoc LANGUAGES C)

add_executable(bintoc bintoc.c)
add_subdirectory(../../extern/athena/extern/zlib zlib)
target_link_libraries(bintoc PRIVATE ${ZLIB_LIBRARIES})

install(TARGETS bintoc DESTINATION bin)
