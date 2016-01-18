include_directories(${LIBPNG_INCLUDE_DIR})
add_executable(packicons packicons.c)
target_link_libraries(packicons ${PNG_LIB} ${ZLIB_LIBRARIES})

unset(INKSCAPE_BIN CACHE)
set(CMAKE_FIND_APPBUNDLE "NEVER")
find_program(INKSCAPE_BIN inkscape)
if(INKSCAPE_BIN STREQUAL "INKSCAPE_BIN-NOTFOUND")
  message(STATUS "Inkscape not found; downloading icons")
  file(DOWNLOAD "https://www.dropbox.com/s/wnj17dwgcsky0o9/icons.bin"
       ${CMAKE_CURRENT_BINARY_DIR}/icons.bin SHOW_PROGRESS)
else()
  message(STATUS "Inkscape found; will render icons locally")
  add_custom_command(OUTPUT icons.bin COMMAND $<TARGET_FILE:packicons>
                     ARGS ${INKSCAPE_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/icons.svg
                     ${CMAKE_CURRENT_BINARY_DIR}/icons.bin
                     MAIN_DEPENDENCY icons.svg COMMENT "Generating icons.bin")
endif()

bintoc(icons.c ${CMAKE_CURRENT_BINARY_DIR}/icons.bin URDE_ICONS)
add_library(UrdeIcons icons.cpp icons.hpp icons.bin icons.c)
