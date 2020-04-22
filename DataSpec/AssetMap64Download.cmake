message(STATUS "64-bit asset name map not found; downloading to '${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap64.bin'")
file(DOWNLOAD "https://axiodl.com/files/AssetNameMap64.dat"
        ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap64.bin SHOW_PROGRESS EXPECTED_HASH SHA1=e49c03c9fff66adccec7af8120dda091636513e2)
