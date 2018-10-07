include_directories(../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../extern/athena/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../extern/boo
        ${CMAKE_CURRENT_SOURCE_DIR}/../extern/boo/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../extern/boo/logvisor/include)
add_library(shaderc_lib shaderc.cpp shaderc.hpp)
add_executable(shaderc main.cpp)
target_link_libraries(shaderc shaderc_lib hecl-compilers glslang soxr xxhash OSDependent OGLCompiler
                      SPIRV glslang-default-resource-limits athena-core logvisor)
if (NOT WIN32)
  target_link_libraries(shaderc pthread)
  if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    target_link_libraries(shaderc execinfo)
  else()
    target_link_libraries(shaderc dl)
  endif()
endif()
if(COMMAND add_sanitizers)
  add_sanitizers(shaderc_lib)
  add_sanitizers(shaderc)
endif()

function(shaderc out)
  if(IS_ABSOLUTE ${out})
    set(theOut ${out})
  else()
    set(theOut ${CMAKE_CURRENT_BINARY_DIR}/${out})
  endif()
  unset(theInsList)
  foreach(in ${ARGN})
    if(IS_ABSOLUTE ${in})
      list(APPEND theInsList ${in})
    else()
      list(APPEND theInsList ${CMAKE_CURRENT_SOURCE_DIR}/${in})
    endif()
  endforeach()
  get_filename_component(outDir ${theOut} DIRECTORY)
  file(MAKE_DIRECTORY ${outDir})
  file(RELATIVE_PATH outRel ${CMAKE_BINARY_DIR} ${theOut})
  add_custom_command(OUTPUT ${theOut}.cpp ${theOut}.hpp
          COMMAND $<TARGET_FILE:shaderc> ARGS -o ${theOut} ${theInsList}
          DEPENDS ${theInsList} shaderc COMMENT "Compiling shader ${outRel}.shader")
endfunction()
