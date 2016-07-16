cmake_policy(SET CMP0053 OLD)


include_directories(${LIBPNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
add_executable(packbadge packbadge.c)
target_link_libraries(packbadge ${PNG_LIB} ${ZLIB_LIBRARIES})

add_custom_command(OUTPUT badge.bin COMMAND $<TARGET_FILE:packbadge>
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/badge.png
                   ${CMAKE_CURRENT_BINARY_DIR}/badge.bin
                   MAIN_DEPENDENCY badge.png COMMENT "Generating badge.bin")

bintoc(badge.c ${CMAKE_CURRENT_BINARY_DIR}/badge.bin URDE_BADGE)
add_library(UrdeBadging
    badge.c badge.bin
    Badging.hpp Badging.cpp)
