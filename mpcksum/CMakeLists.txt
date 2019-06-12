add_executable(mpcksum main.cpp)
target_include_directories(mpcksum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../../Runtime)
target_link_libraries(mpcksum athena-core ${ZLIB_LIBRARIES} lzokay)
