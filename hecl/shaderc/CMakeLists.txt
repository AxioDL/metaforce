cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15...3.20)
project(shaderc LANGUAGES C CXX)

if (MSVC)
  # Shaddup MSVC
  add_compile_definitions(UNICODE=1 _UNICODE=1 __SSE__=1
      _CRT_SECURE_NO_WARNINGS=1 D_SCL_SECURE_NO_WARNINGS=1
      _SCL_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
      _ENABLE_EXTENDED_ALIGNED_STORAGE=1 NOMINMAX=1)
  add_compile_options(/IGNORE:4221
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4018>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4800>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4005>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4311>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4068>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4267>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4244>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4200>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4305>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4067>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4146>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4309>
      $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:/wd4805>
      ${VS_OPTIONS})

  string(REPLACE "/GR " "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REPLACE " /EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_options(
      # Disable exceptions
      $<$<COMPILE_LANGUAGE:CXX>:/EHsc->

      # Disable RTTI
      $<$<COMPILE_LANGUAGE:CXX>:/GR->

      # Enforce various standards compliant behavior.
      $<$<COMPILE_LANGUAGE:CXX>:/permissive->

      # Enable standard volatile semantics.
      $<$<COMPILE_LANGUAGE:CXX>:/volatile:iso>

      # Reports the proper value for the __cplusplus preprocessor macro.
      $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>

      # Use latest C++ standard.
      $<$<COMPILE_LANGUAGE:CXX>:/std:c++latest>
  )

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Flags for MSVC (not clang-cl)
    add_compile_options(
        # Allow constexpr variables to have explicit external linkage.
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:externConstexpr>

        # Assume that new throws exceptions, allowing better code generation.
        $<$<COMPILE_LANGUAGE:CXX>:/Zc:throwingNew>

        # Link-time Code Generation for Release builds
        $<$<CONFIG:Release>:/GL>
    )

    # Link-time Code Generation for Release builds
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /RELEASE /OPT:REF /OPT:ICF /INCREMENTAL:NO /DEBUGTYPE:cv,fixup")
  endif()
else()
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

# two step: --target shaderc, --target install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

add_subdirectory(../../extern/athena athena)
add_subdirectory(../../extern/boo boo)

add_library(hecl-compilers ../lib/Compilers.cpp ../lib/WideStringConvert.cpp)
get_target_property(BOO_INCLUDES boo INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(hecl-compilers PUBLIC ../include ${BOO_INCLUDES})
target_link_libraries(hecl-compilers PUBLIC athena-core logvisor xxhash
    glslang OSDependent OGLCompiler SPIRV glslang-default-resource-limits)

add_library(shaderc_lib shaderc.cpp shaderc.hpp)
target_link_libraries(shaderc_lib PUBLIC hecl-compilers athena-core logvisor)
add_executable(shaderc main.cpp)
target_link_libraries(shaderc shaderc_lib)

if (NOT WIN32)
  target_link_libraries(shaderc pthread)
  if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    target_link_libraries(shaderc execinfo)
  else()
    target_link_libraries(shaderc dl)
  endif()
endif()

install(TARGETS shaderc DESTINATION bin)