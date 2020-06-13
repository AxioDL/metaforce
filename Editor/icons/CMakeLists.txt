if(NOT CMAKE_CROSSCOMPILING)
add_executable(packicons packicons.c)
find_library(M_LIB m)
if(NOT M_LIB)
  unset(M_LIB CACHE)
endif()
target_link_libraries(packicons ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${M_LIB})
target_include_directories(packicons PRIVATE ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

##################
# Package Export #
##################

# Add all targets to the build-tree export set
export(TARGETS packicons FILE "${CMAKE_CURRENT_BINARY_DIR}/urde-packiconsTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE urde-packicons)

# Create the atdnaConfig.cmake
# ... for the build tree
configure_file(urde-packiconsConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/urde-packiconsConfig.cmake" @ONLY)
# ... for the install tree
configure_file(urde-packiconsConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/urde-packiconsConfig.cmake" @ONLY)
# ... for both
configure_file(urde-packiconsConfigVersion.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/urde-packiconsConfigVersion.cmake" @ONLY)

else()
find_package(urde-packicons REQUIRED)
endif()

unset(INKSCAPE_BIN CACHE)
set(CMAKE_FIND_APPBUNDLE "NEVER")
if(WIN32)
  find_program(INKSCAPE_BIN inkscape.exe PATHS
               "$ENV{PROGRAMFILES}/Inkscape"
               "$ENV{ProgramW6432}/Inkscape"
               "$ENV{PROGRAMFILES\(X86\)}/Inkscape")
else()
  set(CMAKE_FIND_APPBUNDLE FIRST)
  find_program(INKSCAPE_BIN inkscape)
endif()
if(NOT INKSCAPE_BIN)
  add_custom_command(OUTPUT icons.bin COMMAND ${CMAKE_COMMAND} ARGS -P
      ${CMAKE_CURRENT_SOURCE_DIR}/IconDownload.cmake)
else()
  if(APPLE)
    get_filename_component(INKSCAPE_DIR ${INKSCAPE_BIN} DIRECTORY)
    if(EXISTS "${INKSCAPE_DIR}/../Resources/bin/inkscape")
      set(INKSCAPE_BIN "${INKSCAPE_DIR}/../Resources/bin/inkscape")
    endif()
  endif()
  message(STATUS "Inkscape found; will render icons locally")
  add_custom_command(OUTPUT icons.bin COMMAND $<TARGET_FILE:packicons>
                     ARGS ${INKSCAPE_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/icons.svg
                     ${CMAKE_CURRENT_BINARY_DIR}/icons.bin
                     MAIN_DEPENDENCY icons.svg COMMENT "Generating icons.bin")
endif()

bintoc(icons_dat.cpp ${CMAKE_CURRENT_BINARY_DIR}/icons.bin URDE_ICONS)
add_library(UrdeIcons icons.cpp icons.hpp icons.bin icons_dat.cpp)
target_link_libraries(UrdeIcons PUBLIC specter)
