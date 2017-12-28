message(STATUS "32-bit asset name map not found; downloading to '${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap32.bin'")
file(DOWNLOAD "https://axiodl.com/AssetNameMap32.dat"
     ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap32.bin SHOW_PROGRESS EXPECTED_HASH SHA1=e1df648c04d4caaf0821266464c672e0c337b6f0)
