include_directories(${LIBPNG_INCLUDE_DIR} ${SQUISH_INCLUDE_DIR})

# Assembles a source/header pair list for use in a DNA library
macro(make_dnalist outlist subdir)
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${subdir}")
  foreach(type ${ARGN})
    get_filename_component(dir ${type} DIRECTORY)
    if(dir)
      file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${subdir}/${dir}")
      set(dir "${dir}/")
    endif()
    get_filename_component(name ${type} NAME)
    atdna("${subdir}/${dir}atdna_${name}.cpp" "${subdir}/${dir}${name}.hpp")
    list(APPEND ${outlist}
         ${subdir}/${dir}atdna_${name}.cpp
         ${subdir}/${dir}${name}.hpp)
  endforeach()
endmacro()

# Assembles source files together for the main DataSpecCommon library
macro(dataspec_add_list rel_path a_list)
  unset(tmp_list)
  foreach(path IN LISTS ${a_list})
    if (IS_ABSOLUTE ${path})
      list(APPEND tmp_list "${path}")
    else()
      list(APPEND tmp_list "${rel_path}/${path}")
    endif()
  endforeach(path)
  set(${a_list} "${tmp_list}")
endmacro(dataspec_add_list)

include_directories(${ZLIB_INCLUDE_DIR} ${LZO_INCLUDE_DIR})

# Each game's DNA library
include(DNACommon/CMakeLists.txt)
include(DNAMP1/CMakeLists.txt)
include(DNAMP2/CMakeLists.txt)
include(DNAMP3/CMakeLists.txt)

# Embed master shader script
bintoc(RetroMasterShader.cpp Blender/RetroMasterShader.py RETRO_MASTER_SHADER)

# Download asset name databases
add_custom_command(OUTPUT AssetNameMap32.bin COMMAND ${CMAKE_COMMAND} ARGS -P
                   ${CMAKE_CURRENT_SOURCE_DIR}/AssetMap32Download.cmake)
bintoc(AssetNameMap32.cpp ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap32.bin ASSET_NAME_MP32)

add_custom_command(OUTPUT AssetNameMap64.bin COMMAND ${CMAKE_COMMAND} ARGS -P
                   ${CMAKE_CURRENT_SOURCE_DIR}/AssetMap64Download.cmake)
bintoc(AssetNameMap64.cpp ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap64.bin ASSET_NAME_MP64)

# Each game's DataSpec implementation
add_library(RetroDataSpec
            SpecBase.cpp
            ${DNACOMMON_SOURCES}
            SpecMP1.cpp
            ${DNAMP1_SOURCES}
            ${ScriptObjectsMP1_SOURCES}
            SpecMP2.cpp
            ${DNAMP2_SOURCES}
            SpecMP3.cpp
            ${DNAMP3_SOURCES}
            Blender/BlenderSupport.hpp
            Blender/BlenderSupport.cpp
            Blender/RetroMasterShader.py
            AssetNameMap.hpp
            AssetNameMap.cpp
            AssetNameMap32.bin AssetNameMap32.cpp
            AssetNameMap64.bin AssetNameMap64.cpp
            RetroMasterShader.cpp)
if(COMMAND add_sanitizers)
  add_sanitizers(RetroDataSpec)
endif()
if(COMMAND cotire)
  cotire(RetroDataSpec)
endif()
