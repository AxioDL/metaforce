include_directories(${CMAKE_CURRENT_SOURCE_DIR} ./../Runtime)

add_executable(mpcksum main.cpp)

target_link_libraries(mpcksum
    athena-core athena-libyaml z lzokay)
