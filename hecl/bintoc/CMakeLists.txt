cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15...3.20)
project(bintoc LANGUAGES C)

add_executable(bintoc bintoc.c)
add_subdirectory(../../extern/athena/extern/zlib zlib EXCLUDE_FROM_ALL)
target_link_libraries(bintoc PRIVATE ${ZLIB_LIBRARIES})
target_include_directories(bintoc PRIVATE ${ZLIB_INCLUDE_DIR})

install(TARGETS bintoc DESTINATION bin)
