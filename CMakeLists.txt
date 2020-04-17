cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)

if(APPLE)
  # Shaddup Xcode
  function(add_executable TARGET)
    _add_executable(${TARGET} ${ARGN})
    set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_WARNING_CFLAGS "")
  endfunction()
  function(add_library TARGET)
    _add_library(${TARGET} ${ARGN})
    list(GET ARGV 1 ARG1)
    if(NOT ${ARG1} STREQUAL INTERFACE AND NOT ${ARG1} STREQUAL ALIAS)
      set_target_properties(${TARGET} PROPERTIES XCODE_ATTRIBUTE_WARNING_CFLAGS "")
    endif()
  endfunction()
endif()

if(APPLE AND NOT CMAKE_OSX_SYSROOT)
  # If the Xcode SDK is lagging behind system version, CMake needs this done first
  execute_process(COMMAND xcrun --sdk macosx --show-sdk-path
                  OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

option(URDE_CROSSCOMPILING "Don't build tools; attempt package import" OFF)
if (URDE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING On)
endif()

if(CMAKE_CROSSCOMPILING)
  set(HAVE_WORDS_BIGENDIAN_EXITCODE 0 CACHE INTEGER "Makes soxr happy" FORCE)
endif()

project(urde VERSION 0.1.0)

# MSVC has a "latest" flag, which always uses the newest standard
# when available. GCC and Clang posess no such flag, and must be
# manually enforced. CMake, curiously, also doesn't have a "latest"
# standard flag either.
if (NOT MSVC)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force shared libs off" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "Force static libs on" FORCE)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sanitizers-cmake/cmake")
find_package(Sanitizers)

if (NX)
  set(URDE_VECTOR_ISA "neon")
else()
  set(URDE_VECTOR_ISA "sse2" CACHE STRING "Vector ISA to build for (sse2, sse3, sse41, avx, avx2)")
endif()

if(MSVC)
  if(${URDE_VECTOR_ISA} STREQUAL "avx2")
    add_compile_options(/arch:AVX2)
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "avx")
    add_compile_options(/arch:AVX)
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with AVX Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse41")
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with SSE4.1 Vector ISA")
  else()
    message(STATUS "Building with SSE2 Vector ISA")
  endif()

  if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(VS_OPTIONS "/MP")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT urde)
  endif()

  # Shaddup MSVC
  add_compile_definitions(UNICODE=1 _UNICODE=1 __SSE__=1
      _CRT_SECURE_NO_WARNINGS=1 D_SCL_SECURE_NO_WARNINGS=1
      _SCL_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_WARNINGS=1
      _ENABLE_EXTENDED_ALIGNED_STORAGE=1 NOMINMAX=1)
  add_compile_options(/IGNORE:4221 /wd4018 /wd4800 /wd4005 /wd4311 /wd4068
      /wd4267 /wd4244 /wd4200 /wd4305 /wd4067 /wd4146 /wd4309 /wd4805 ${VS_OPTIONS})

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
  add_compile_definitions(FMT_EXCEPTIONS=0 _HAS_EXCEPTIONS=0)

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Flags for MSVC (not clang-cl)
    add_compile_options(
      # Allow constexpr variables to have explicit external linkage.
      $<$<COMPILE_LANGUAGE:CXX>:/Zc:externConstexpr>

      # Assume that new throws exceptions, allowing better code generation.
      $<$<COMPILE_LANGUAGE:CXX>:/Zc:throwingNew>

      # Link-time Code Generation for Release builds
      $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:/GL>
    )

    # Link-time Code Generation for Release builds
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
    set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "/LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /DEBUGTYPE:cv,fixup")
  endif()

else()
  if(${URDE_VECTOR_ISA} STREQUAL "native")
    add_compile_options(-march=native)
    message(STATUS "Building with native ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "avx2")
    add_compile_options(-mavx2)
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "avx")
    add_compile_options(-mavx)
    message(STATUS "Building with AVX Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse41")
    add_compile_options(-msse4.1)
    message(STATUS "Building with SSE4.1 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse3")
    add_compile_options(-msse3)
    message(STATUS "Building with SSE3 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse2")
    add_compile_options(-msse2)
    message(STATUS "Building with SSE2 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "neon")
    message(STATUS "Building with NEON Vector ISA")
  else()
    message(STATUS "Building with x87 Vector ISA")
  endif()

  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fno-plt HAS_NO_PLT)
  if (HAS_NO_PLT)
    add_compile_options(-fno-plt)
  endif()

  if(URDE_MSAN)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++> -fsanitize=memory
                        -fsanitize-memory-track-origins -fsanitize-recover=all)
  endif()
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
                      $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
                      -Wall -Wno-multichar -Werror=implicit-fallthrough -Wno-unknown-warning-option
                      -Wno-lto-type-mismatch -Wno-unused-variable -Wno-unused-private-field
                      -Wno-unused-function -Wno-sign-compare -Wno-unknown-pragmas -Werror)
  add_compile_definitions(FMT_EXCEPTIONS=0)

  if(APPLE)
    add_compile_options(-Wno-error=deprecated-declarations
                        $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:-flto=thin>)
  endif()

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
      # This is required to summarize std::string
      add_compile_options(-fno-limit-debug-info)
    endif()
    option(USE_LD_LLD "Link with LLD" ON)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    option(USE_LD_GOLD "Link with GNU Gold" ON)
  endif()

  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)
  if(LTO_SUPPORTED AND ("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"))
    option(USE_LTO "Enable LTO" ON)
  else()
    option(USE_LTO "Enable LTO" OFF)
  endif()
else()
  option(USE_LD_LLD "Link with LLD" OFF)
  option(USE_LD_GOLD "Link with GNU Gold" OFF)
  option(USE_LTO "Enable LTO" OFF)
endif()
if(USE_LD_LLD)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=lld -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
  if("${LD_VERSION}" MATCHES "LLD")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
    if(USE_LTO)
      add_compile_options(-flto=thin)
      add_link_options(-flto=thin)
      message(STATUS "LLD linker enabled with LTO.")
    else()
      message(STATUS "LLD linker enabled.")
    endif()
    set(USE_LD_GOLD OFF)
  else()
    message(WARNING "LLD linker isn't available, using the default system linker.")
    set(USE_LD_LLD OFF)
  endif()
endif()
if(USE_LD_GOLD)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
  if("${LD_VERSION}" MATCHES "GNU gold")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    if(USE_LTO)
      add_compile_options(-flto=thin)
      add_link_options(-flto=thin)
      message(STATUS "GNU gold linker enabled with LTO.")
    else()
      message(STATUS "GNU gold linker enabled.")
    endif()
    set(USE_LD_LLD OFF)
  else()
    message(WARNING "GNU gold linker isn't available, using the default system linker.")
    set(USE_LD_GOLD OFF)
  endif()
endif()

# Add discord-rpc here
if(NOT GEKKO AND NOT NX)
  set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/discord-rpc)
  if (NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/fake-prefix)
  endif()
  add_subdirectory(discord-rpc/src)
  target_include_directories(discord-rpc PRIVATE rapidjson/include PUBLIC discord-rpc/include)
endif()

add_subdirectory(nod)

set(HECL_DLPACKAGE ${URDE_DLPACKAGE})

set(DATA_SPEC_LIBS RetroDataSpec)
set(HECL_DATASPEC_DECLS
"/* RetroCommon specs */
namespace DataSpec
{
    extern hecl::Database::DataSpecEntry SpecEntMP1;
    extern hecl::Database::DataSpecEntry SpecEntMP1PC;
    extern hecl::Database::DataSpecEntry SpecEntMP1ORIG;
    extern hecl::Database::DataSpecEntry SpecEntMP2;
    extern hecl::Database::DataSpecEntry SpecEntMP2PC;
    extern hecl::Database::DataSpecEntry SpecEntMP2ORIG;
    extern hecl::Database::DataSpecEntry SpecEntMP3;
    extern hecl::Database::DataSpecEntry SpecEntMP3PC;
    extern hecl::Database::DataSpecEntry SpecEntMP3ORIG;
}")
set(HECL_DATASPEC_PUSHES
"    /* RetroCommon */
    hecl::Database::DATA_SPEC_REGISTRY.reserve(9);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP1);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP1PC);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP1ORIG);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP2);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP2PC);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP2ORIG);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP3);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP3PC);
    hecl::Database::DATA_SPEC_REGISTRY.push_back(&DataSpec::SpecEntMP3ORIG);")

add_subdirectory(hecl/shaderc)
include(hecl/ApplicationTools.cmake)
add_subdirectory(specter/shaders)
add_subdirectory(Shaders)
add_subdirectory(hecl)
target_include_directories(hecl-full PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(hecl-light PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(hecl-full PRIVATE zeus nod)
target_link_libraries(hecl-light PRIVATE zeus nod)

if(NOT TARGET bintoc)
  # Use native if cross compiling
  find_package(hecl-bintoc REQUIRED)
endif()

bintoc(CModelShaders.common.glsl.cpp Shaders/CModelShaders.common.glsl CMODELSHADERS_COMMON_GLSL)
bintoc(CModelShaders.vert.glsl.cpp Shaders/CModelShaders.vert.glsl CMODELSHADERS_VERT_GLSL)
bintoc(CModelShaders.frag.glsl.cpp Shaders/CModelShaders.frag.glsl CMODELSHADERS_FRAG_GLSL)
bintoc(CModelShaders.common.hlsl.cpp Shaders/CModelShaders.common.hlsl CMODELSHADERS_COMMON_HLSL)
bintoc(CModelShaders.vert.hlsl.cpp Shaders/CModelShaders.vert.hlsl CMODELSHADERS_VERT_HLSL)
bintoc(CModelShaders.frag.hlsl.cpp Shaders/CModelShaders.frag.hlsl CMODELSHADERS_FRAG_HLSL)
bintoc(CModelShaders.common.metal.cpp Shaders/CModelShaders.common.metal CMODELSHADERS_COMMON_METAL)
bintoc(CModelShaders.vert.metal.cpp Shaders/CModelShaders.vert.metal CMODELSHADERS_VERT_METAL)
bintoc(CModelShaders.frag.metal.cpp Shaders/CModelShaders.frag.metal CMODELSHADERS_FRAG_METAL)
add_library(CModelShaders
    CModelShaders.common.glsl.cpp CModelShaders.vert.glsl.cpp CModelShaders.frag.glsl.cpp
    CModelShaders.common.hlsl.cpp CModelShaders.vert.hlsl.cpp CModelShaders.frag.hlsl.cpp
    CModelShaders.common.metal.cpp CModelShaders.vert.metal.cpp CModelShaders.frag.metal.cpp)
target_link_libraries(shader_CModelShaders PUBLIC CModelShaders)

if(NOT TARGET atdna)
  # Import native atdna if cross-compiling
  find_package(atdna REQUIRED)
  if(NOT TARGET atdna)
    message(FATAL_ERROR "atdna required for building URDE; please verify LLVM installation")
  endif()
endif()

add_subdirectory(amuse)
add_subdirectory(specter)
target_include_directories(specter PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(specter PRIVATE nod)
add_subdirectory(assetnameparser)
add_compile_definitions(URDE_ZIP_INPUT_STREAM=1) # Enable CZipInputStream now that zlib header is known
add_subdirectory(DataSpec)
add_subdirectory(kabufuda)
add_subdirectory(Editor)

add_subdirectory(jbus)
set(JBUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jbus/include)

set(CLIENT_SOURCES
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryBase.hpp
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryBase.cpp
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryMP1.hpp
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryMP1.cpp)
add_subdirectory(NESEmulator)
add_subdirectory(Runtime)
add_subdirectory(mpcksum)
add_subdirectory(gbalink)
add_subdirectory(visigen)

add_dependencies(hecl visigen)

if (NOT WINDOWS_STORE AND NOT NX)
  find_package(Qt5Widgets)
  if (Qt5Widgets_FOUND)
    message(STATUS "Qt5 found, hecl-gui will be built")
    add_subdirectory(hecl-gui)
  else()
    message(STATUS "Qt5 not found, hecl-gui will not be built")
  endif()
endif()

unset(GIT_EXECUTABLE CACHE)
find_package(Git)
if(GIT_FOUND)
    # Get the current working branch
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE )

    # Get the latest abbreviated commit hash of the working branch
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%h WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE
        GIT_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE
        GIT_COMMIT_HASH_FULL OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%ad WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE
        GIT_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    message(STATUS "Unable to find GIT, commit information will not be available")
    set(GIT_BRANCH "")
    set(GIT_COMMIT_HASH "")
    set(GIT_COMMIT_HASH_FULL "")
    set(GIT_COMMIT_DATE "")
endif()

configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_BINARY_DIR}/version.h)
