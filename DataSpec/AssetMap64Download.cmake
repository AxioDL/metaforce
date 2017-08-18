message(STATUS "64-bit asset name map not found; downloading to '${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap64.bin'")
file(DOWNLOAD "https://www.dropbox.com/s/o1pp3ctu5dyhfwx/AssetNameMap64.dat"
        ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap64.bin SHOW_PROGRESS EXPECTED_HASH SHA1=335abe49d4848ddc7bb2d623acc76918365bc705)
