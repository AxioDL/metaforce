# Assembles a source/header pair list for use in a DNA library
macro(make_dnalist)
  file(RELATIVE_PATH subdir "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_LIST_DIR}")
  set(CMAKE_CURRENT_LIST_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/${subdir}")
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_BINARY_DIR}")
  foreach(type ${ARGN})
    get_filename_component(dir ${type} DIRECTORY)
    if(dir)
      file(MAKE_DIRECTORY "${CMAKE_CURRENT_LIST_BINARY_DIR}/${dir}")
      set(dir "${dir}/")
    endif()
    get_filename_component(name ${type} NAME)
    list(APPEND DNA_SOURCES "${subdir}/${dir}atdna_${name}.cpp")
    list(APPEND DNA_HEADERS "${subdir}/${dir}${name}.hpp")
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

# Each game's DNA library
unset(DNA_SOURCES)
unset(DNA_HEADERS)
include(DNACommon/CMakeLists.txt)
include(DNAMP1/CMakeLists.txt)
include(DNAMP2/CMakeLists.txt)
include(DNAMP3/CMakeLists.txt)

# Embed master shader script
bintoc(RetroMasterShader.cpp Blender/RetroMasterShader.py RETRO_MASTER_SHADER)

# Download asset name databases
add_custom_command(OUTPUT AssetNameMap32.bin COMMAND ${CMAKE_COMMAND} ARGS -P
                   ${CMAKE_CURRENT_SOURCE_DIR}/AssetMap32Download.cmake)
bintoc_compress(AssetNameMap32.cpp ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap32.bin ASSET_NAME_MP32)

add_custom_command(OUTPUT AssetNameMap64.bin COMMAND ${CMAKE_COMMAND} ARGS -P
                   ${CMAKE_CURRENT_SOURCE_DIR}/AssetMap64Download.cmake)
bintoc_compress(AssetNameMap64.cpp ${CMAKE_CURRENT_BINARY_DIR}/AssetNameMap64.bin ASSET_NAME_MP64)

# Each game's DataSpec implementation
add_library(RetroDataSpec
            SpecBase.cpp
            ${DNACOMMON_SOURCES}
            SpecMP1.cpp
            ${DNAMP1_SOURCES}
            ${ScriptObjectsMP1_SOURCES}
            ${DNAMP1_SFX_SOURCES}
            SpecMP2.cpp
            ${DNAMP2_SOURCES}
            SpecMP3.cpp
            ${DNAMP3_SOURCES}
            Blender/BlenderSupport.hpp
            Blender/BlenderSupport.cpp
            Blender/RetroMasterShader.py
            AssetNameMap.hpp
            AssetNameMap.cpp
            RetroMasterShader.cpp)
add_library(AssetNameMap
        AssetNameMap32.bin AssetNameMap32.cpp
        AssetNameMap64.bin AssetNameMap64.cpp)
add_library(AssetNameMapNull
        AssetNameMapNull.cpp)

get_target_property(HECL_INCLUDES hecl-full INCLUDE_DIRECTORIES)
target_include_directories(RetroDataSpec PUBLIC ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR}
                           ${HECL_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
target_link_libraries(RetroDataSpec PUBLIC amuse zeus nod specter squish ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} lzokay logvisor)
if(COMMAND add_sanitizers)
  add_sanitizers(RetroDataSpec)
endif()

# Resolve all DNA sources into target
list(LENGTH DNA_SOURCES count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
  list(GET DNA_SOURCES ${i} src)
  list(GET DNA_HEADERS ${i} header)
  target_atdna(RetroDataSpec ${src} ${header})
endforeach()

add_custom_target(genexdebug COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_PROPERTY:RetroDataSpec,INCLUDE_DIRECTORIES>")
