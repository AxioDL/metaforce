add_library(xxhash hecl-xxhash.c hecl-xxhash.h)
target_include_directories(xxhash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
