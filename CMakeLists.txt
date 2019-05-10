cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17

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
    if(NOT ${ARG1} STREQUAL INTERFACE)
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

project(urde)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Force shared libs off" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "Force static libs on" FORCE)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/sanitizers-cmake/cmake" "${CMAKE_SOURCE_DIR}/cotire/CMake" ${CMAKE_MODULE_PATH})
find_package(Sanitizers)

# Don't cotire when using clang-cl
if(NOT URDE_MSAN AND (NOT MSVC OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC"))
  include(cotire)
endif()

if (NX)
  set(URDE_VECTOR_ISA "neon")
else()
  set(URDE_VECTOR_ISA "sse2" CACHE STRING "Vector ISA to build for (sse2, sse3, sse41, avx, avx2)")
endif()

if(MSVC)
  if(${URDE_VECTOR_ISA} STREQUAL "avx2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2 -D__SSE4_1__=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX2 -D__SSE4_1__=1")
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "avx")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX -D__SSE4_1__=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX -D__SSE4_1__=1")
    message(STATUS "Building with AVX Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse41")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__SSE4_1__=1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__SSE4_1__=1")
    message(STATUS "Building with SSE4.1 Vector ISA")
  else()
    message(STATUS "Building with SSE2 Vector ISA")
  endif()

  if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(VS_DEFINES "/MP")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT urde)
  endif()

  # Shaddup MSVC
  add_definitions(-DUNICODE=1 -D_UNICODE=1 -D__SSE__=1
      -D_CRT_SECURE_NO_WARNINGS=1 -DD_SCL_SECURE_NO_WARNINGS=1
      -D_SCL_SECURE_NO_DEPRECATE=1 -D_CRT_NONSTDC_NO_WARNINGS=1
      /IGNORE:4221 /wd4018 /wd4800 /wd4005 /wd4311 /wd4068
      /wd4267 /wd4244 /wd4200 /wd4305 /wd4067 /wd4146 /wd4309 /wd4805
      -D_ENABLE_EXTENDED_ALIGNED_STORAGE=1 ${VS_DEFINES})

  if(WINDOWS_STORE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /AI\"$ENV{PROGRAMFILES\(X86\)}/Microsoft Visual Studio/2017/Community/Common7/IDE/VC/vcpackages\" /AI\"$ENV{PROGRAMFILES\(X86\)}/Windows Kits/10/UnionMetadata\"")
    set(HAVE_WORDS_BIGENDIAN_EXITCODE 0)
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

  # Link-time Code Generation for Release builds (excluding clang-cl)
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_C_FLAGS_RELEASE "/DNDEBUG /O2 /Oy /GL /Gy /MD")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "/DNDEBUG /Zi /O2 /Oy- /GL /Gy /MD")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "/LTCG")
    set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "/LTCG")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /DEBUGTYPE:cv,fixup")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
  endif()

else()
  if(${URDE_VECTOR_ISA} STREQUAL "avx2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "avx")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx")
    message(STATUS "Building with AVX Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse41")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4.1")
    message(STATUS "Building with SSE4.1 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse3")
    message(STATUS "Building with SSE3 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "sse2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
    message(STATUS "Building with SSE2 Vector ISA")
  elseif(${URDE_VECTOR_ISA} STREQUAL "neon")
    message(STATUS "Building with NEON Vector ISA")
  else()
    message(STATUS "Building with x87 Vector ISA")
  endif()

  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fno-plt HAS_NO_PLT)
  if (HAS_NO_PLT)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-plt")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-plt")
  endif()

  if(URDE_MSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -stdlib=libc++ -fsanitize=memory -fsanitize-memory-track-origins -fsanitize-recover=all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
        -fsanitize=memory -fsanitize-memory-track-origins -fsanitize-recover=all")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-multichar -Werror=implicit-fallthrough \
      -Wno-unknown-warning-option -Wno-lto-type-mismatch -Wno-unused-variable -Wno-unused-private-field \
      -Wno-unused-function -fno-exceptions -fno-rtti -Werror")

  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto=thin")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -flto=thin")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=thin")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -flto=thin")
  endif()

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
include_directories(/usr/local/include)
link_directories(/usr/local/lib)
endif()

if(("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
   AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  option(USE_LD_GOLD "Link with GNU Gold and enable LTO" ON)
else()
  option(USE_LD_GOLD "Link with GNU Gold and enable LTO" OFF)
endif()
if(USE_LD_GOLD AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
    if("${LD_VERSION}" MATCHES "GNU gold")
       set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
       set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
       set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto=thin")
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto=thin")
       message(STATUS "GNU gold linker enabled.")
    else()
       message(WARNING "GNU gold linker isn't available, using the default system linker.")
       set(USE_LD_GOLD OFF)
    endif()
endif()

# Add discord-rpc here
if(NOT GEKKO AND NOT NX)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)
  set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/discord-rpc)
  if (NOT CMAKE_INSTALL_LIBDIR)
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/fake-prefix)
  endif()
  add_subdirectory(discord-rpc/src)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/discord-rpc/include)
endif()

add_subdirectory(nod)
set(NOD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nod/include)

set(HECL_DLPACKAGE ${URDE_DLPACKAGE})

set(BOO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hecl/extern/boo/include)
set(HECL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hecl/include
                     ${CMAKE_CURRENT_SOURCE_DIR}/hecl/blender)
set(SPECTER_INCLUDE_DIR specter/include specter/freetype2/include)
set(ZEUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/specter/zeus/include)

set(DATA_SPEC_LIBS RetroDataSpec amuse)
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
add_definitions(-DZE_ATHENA_TYPES=1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SPECTER_INCLUDE_DIR} ${NOD_INCLUDE_DIR}
                    ${ZEUS_INCLUDE_DIR})
add_subdirectory(hecl/shaderc)
include(hecl/ApplicationTools.cmake)
add_subdirectory(specter/shaders)
add_subdirectory(Shaders)
add_subdirectory(hecl)

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
add_library(CModelShaders
    CModelShaders.common.glsl.cpp CModelShaders.vert.glsl.cpp CModelShaders.frag.glsl.cpp
    CModelShaders.common.hlsl.cpp CModelShaders.vert.hlsl.cpp CModelShaders.frag.hlsl.cpp)

if(NOT TARGET atdna)
  # Import native atdna if cross-compiling
  find_package(atdna REQUIRED)
  if(NOT TARGET atdna)
    message(FATAL_ERROR "atdna required for building URDE; please verify LLVM installation")
  endif()
endif()

add_definitions(${BOO_SYS_DEFINES})
include_directories(${BOO_SYS_INCLUDES})
add_subdirectory(amuse)
add_subdirectory(specter)
add_subdirectory(assetnameparser)
include_directories(${ATHENA_INCLUDE_DIR} ${LOGVISOR_INCLUDE_DIR} ${HECL_INCLUDE_DIR}
                    ${BOO_INCLUDE_DIR} ${AMUSE_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/NESEmulator
                    ${ZLIB_INCLUDE_DIR})
add_definitions(-DURDE_ZIP_INPUT_STREAM=1) # Enable CZipInputStream now that zlib header is known
add_subdirectory(NESEmulator)
add_subdirectory(DataSpec)
add_subdirectory(kabufuda)
include_directories(${KABUFUDA_INCLUDE_DIR})
add_subdirectory(Editor)

add_subdirectory(jbus)
set(JBUS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/jbus/include)

set(CLIENT_SOURCES
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryBase.hpp
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryBase.cpp
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryMP1.hpp
    ${CMAKE_SOURCE_DIR}/Editor/ProjectResourceFactoryMP1.cpp)
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

configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_SOURCE_DIR}/version.h)
