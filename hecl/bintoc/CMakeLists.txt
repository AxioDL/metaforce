if(NOT CMAKE_CROSSCOMPILING)
add_executable(bintoc bintoc.c)
macro(bintoc out in sym)
  if(IS_ABSOLUTE ${out})
    set(theOut ${out})
  else()
    set(theOut ${CMAKE_CURRENT_BINARY_DIR}/${out})
  endif()
  if(IS_ABSOLUTE ${in})
    set(theIn ${in})
  else()
    set(theIn ${CMAKE_CURRENT_SOURCE_DIR}/${in})
  endif()
  add_custom_command(OUTPUT ${theOut}
                     COMMAND $<TARGET_FILE:bintoc> ARGS ${theIn} ${theOut} ${sym}
                     DEPENDS ${theIn} bintoc)
endmacro()

##################
# Package Export #
##################

# Add all targets to the build-tree export set
export(TARGETS bintoc FILE "${CMAKE_CURRENT_BINARY_DIR}/hecl-bintocTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE hecl-bintoc)

# Create the atdnaConfig.cmake
# ... for the build tree
configure_file(hecl-bintocConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/hecl-bintocConfig.cmake" @ONLY)
# ... for the install tree
configure_file(hecl-bintocConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/hecl-bintocConfig.cmake" @ONLY)
# ... for both
configure_file(hecl-bintocConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/hecl-bintocConfigVersion.cmake" @ONLY)

else()
# Use native if cross compiling
find_package(hecl-bintoc REQUIRED)
endif()
