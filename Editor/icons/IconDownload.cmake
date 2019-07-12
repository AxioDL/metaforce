message(STATUS "Inkscape not found; downloading icons to '${CMAKE_CURRENT_BINARY_DIR}/icons.bin'")
file(DOWNLOAD "https://axiodl.com/files/icons.bin"
     ${CMAKE_CURRENT_BINARY_DIR}/icons.bin SHOW_PROGRESS EXPECTED_HASH SHA1=412baa3b12163a89b306ebfef2a395762105a8d4)