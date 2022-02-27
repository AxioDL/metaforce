add_library(xxhash xxhash_impl.c)
target_include_directories(xxhash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(xxhash INTERFACE XXH_STATIC_LINKING_ONLY)
