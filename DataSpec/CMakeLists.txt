include_directories(${HECL_INCLUDE_DIR} ${NOD_LIB_INCLUDE_DIR} ${LIBPNG_INCLUDE_DIR}
                    ${SQUISH_INCLUDE_DIR})

# Magic ingredient
find_package(atdna REQUIRED)

# Assembles a source/header pair list for use in a DNA library
macro(make_dnalist outlist)
  foreach(type ${ARGN})
    atdna(atdna_${type}.cpp ${type}.hpp)
    list(APPEND ${outlist} atdna_${type}.cpp ${type}.hpp)
  endforeach()
endmacro()

include_directories(${ZLIB_INCLUDE_DIR} ${LZO_INCLUDE_DIR})

# Each game's DNA library
add_subdirectory(DNACommon)
add_subdirectory(DNAMP1)
add_subdirectory(DNAMP2)
add_subdirectory(DNAMP3)

# Each game's DataSpec implementation
add_library(RetroDataSpec
            SpecBase.cpp
            SpecMP1.cpp
            SpecMP2.cpp
            SpecMP3.cpp)
