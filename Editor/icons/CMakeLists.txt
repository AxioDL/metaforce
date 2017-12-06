if(NOT WINDOWS_STORE)
include_directories(${LIBPNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
add_executable(packicons packicons.c)
target_link_libraries(packicons ${PNG_LIB} ${ZLIB_LIBRARIES})

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
  message(STATUS "Inkscape found; will render icons locally")
  add_custom_command(OUTPUT icons.bin COMMAND $<TARGET_FILE:packicons>
                     ARGS ${INKSCAPE_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/icons.svg
                     ${CMAKE_CURRENT_BINARY_DIR}/icons.bin
                     MAIN_DEPENDENCY icons.svg COMMENT "Generating icons.bin")
endif()
else()
  add_custom_command(OUTPUT icons.bin COMMAND ${CMAKE_COMMAND} ARGS -P
      ${CMAKE_CURRENT_SOURCE_DIR}/IconDownload.cmake)
endif()

bintoc(icons.c ${CMAKE_CURRENT_BINARY_DIR}/icons.bin URDE_ICONS)
add_library(UrdeIcons icons.cpp icons.hpp icons.bin icons.c)
