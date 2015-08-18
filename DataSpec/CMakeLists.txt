include_directories(${LIBPNG_INCLUDE_DIR} ${SQUISH_INCLUDE_DIR})

# Magic ingredient
find_package(atdna REQUIRED)

# Assembles a source/header pair list for use in a DNA library
macro(make_dnalist outlist)
  foreach(type ${ARGN})
    get_filename_component(dir ${type} DIRECTORY)
    get_filename_component(name ${type} NAME)
    atdna(${dir}/atdna_${name}.cpp ${dir}/${name}.hpp)
    list(APPEND ${outlist}
         ${CMAKE_CURRENT_BINARY_DIR}/${dir}/atdna_${name}.cpp
         ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${name}.hpp)
  endforeach()
endmacro()

include_directories(${ZLIB_INCLUDE_DIR} ${LZO_INCLUDE_DIR})

# Each game's DNA library
add_subdirectory(DNACommon)
add_subdirectory(DNAMP1)
add_subdirectory(DNAMP2)
add_subdirectory(DNAMP3)

# Embed master shader script
bintoc(RetroMasterShader.c Blender/RetroMasterShader.py RETRO_MASTER_SHADER)

# Each game's DataSpec implementation
add_library(RetroDataSpec
            SpecBase.cpp
            SpecMP1.cpp
            SpecMP2.cpp
            SpecMP3.cpp
            Blender/BlenderSupport.hpp
            Blender/BlenderSupport.cpp
            Blender/RetroMasterShader.py
            RetroMasterShader.c)
