project(propparse)
cmake_minimum_required(VERSION 3.0)

add_subdirectory(extern/tinyxml2)
include_directories(extern/tinyxml2)
add_executable(propparse
        main.cpp
        Parser.hpp Parser.cpp)
target_link_libraries(propparse tinyxml2 AthenaCore AthenaLibYaml z lzo2)
