message(STATUS "Inkscape not found; downloading icons to '${CMAKE_CURRENT_BINARY_DIR}/icons.bin'")
file(DOWNLOAD "https://www.dropbox.com/s/wnj17dwgcsky0o9/icons.bin"
     ${CMAKE_CURRENT_BINARY_DIR}/icons.bin SHOW_PROGRESS)