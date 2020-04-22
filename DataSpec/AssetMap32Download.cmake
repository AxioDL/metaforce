message(STATUS "32-bit asset name map not found; downloading to '${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap32.bin'")
file(DOWNLOAD "https://axiodl.com/files/AssetNameMap32.dat"
     ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap32.bin SHOW_PROGRESS EXPECTED_HASH SHA1=90b4e941c192eef41c81e60314f348bc787d1336)
