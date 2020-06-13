if(NOT CMAKE_CROSSCOMPILING)

add_executable(packbadge packbadge.c)
find_library(M_LIB m)
if(NOT M_LIB)
  unset(M_LIB CACHE)
endif()
target_link_libraries(packbadge ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${M_LIB})
target_include_directories(packbadge PRIVATE ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

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

bintoc(badge.cpp ${CMAKE_CURRENT_BINARY_DIR}/badge.bin URDE_BADGE)
add_library(UrdeBadging
    badge.cpp badge.bin
    Badging.hpp Badging.cpp)
target_link_libraries(UrdeBadging PUBLIC specter)
