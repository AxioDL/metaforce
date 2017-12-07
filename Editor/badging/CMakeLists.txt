if(NOT CMAKE_CROSSCOMPILING)

include_directories(${LIBPNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
add_executable(packbadge packbadge.c)
target_link_libraries(packbadge ${PNG_LIB} ${ZLIB_LIBRARIES})

##################
# Package Export #
##################

# Add all targets to the build-tree export set
export(TARGETS packbadge FILE "${CMAKE_CURRENT_BINARY_DIR}/urde-packbadgeTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE urde-packbadge)

# Create the atdnaConfig.cmake
# ... for the build tree
configure_file(urde-packbadgeConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/urde-packbadgeConfig.cmake" @ONLY)
# ... for the install tree
configure_file(urde-packbadgeConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/urde-packbadgeConfig.cmake" @ONLY)
# ... for both
configure_file(urde-packbadgeConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/urde-packbadgeConfigVersion.cmake" @ONLY)

else()
find_package(urde-packbadge REQUIRED)
endif()

add_custom_command(OUTPUT badge.bin COMMAND $<TARGET_FILE:packbadge>
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/badge.png
                   ${CMAKE_CURRENT_BINARY_DIR}/badge.bin
                   MAIN_DEPENDENCY badge.png COMMENT "Generating badge.bin")

bintoc(badge.c ${CMAKE_CURRENT_BINARY_DIR}/badge.bin URDE_BADGE)
add_library(UrdeBadging
    badge.c badge.bin
    Badging.hpp Badging.cpp)
