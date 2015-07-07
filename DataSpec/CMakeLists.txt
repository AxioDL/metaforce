include_directories(${HECL_INCLUDE_DIR} ${NOD_LIB_INCLUDE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(DNACommon)
add_subdirectory(DNAMP1)
add_subdirectory(DNAMP2)
add_subdirectory(DNAMP3)

add_library(RetroDataSpec
            SpecBase.cpp
            SpecMP1.cpp
            SpecMP2.cpp
            SpecMP3.cpp)
target_link_libraries(RetroDataSpec
                      DNAMP1
                      DNAMP3)
