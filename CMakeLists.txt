cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15...3.20)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
      "Build type options: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif ()

# obtain revision info from git
find_package(Git)
if (GIT_FOUND)
  # make sure version information gets re-run when the current Git HEAD changes
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --git-path HEAD
          OUTPUT_VARIABLE metaforce_git_head_filename
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${metaforce_git_head_filename}")

  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --symbolic-full-name HEAD
          OUTPUT_VARIABLE metaforce_git_head_symbolic
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
          COMMAND ${GIT_EXECUTABLE} rev-parse --git-path ${metaforce_git_head_symbolic}
          OUTPUT_VARIABLE metaforce_git_head_symbolic_filename
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${metaforce_git_head_symbolic_filename}")

  # defines METAFORCE_WC_REVISION
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
          OUTPUT_VARIABLE METAFORCE_WC_REVISION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  # defines METAFORCE_WC_DESCRIBE
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} describe --tag --long --dirty
          OUTPUT_VARIABLE METAFORCE_WC_DESCRIBE
          OUTPUT_STRIP_TRAILING_WHITESPACE)

  # remove hash (and trailing "-0" if needed) from description
  string(REGEX REPLACE "(-0)?-[^-]+((-dirty)?)$" "\\2" METAFORCE_WC_DESCRIBE "${METAFORCE_WC_DESCRIBE}")

  # defines METAFORCE_WC_BRANCH
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
          OUTPUT_VARIABLE METAFORCE_WC_BRANCH
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  # defines METAFORCE_WC_DATE
  execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${GIT_EXECUTABLE} log -1 --format=%ad
          OUTPUT_VARIABLE METAFORCE_WC_DATE
          OUTPUT_STRIP_TRAILING_WHITESPACE)
else ()
  message(STATUS "Unable to find git, commit information will not be available")
endif ()

if (METAFORCE_WC_DESCRIBE)
  string(REGEX REPLACE "v([0-9]+)\.([0-9]+)\.([0-9]+)\-([0-9]+).*" "\\1.\\2.\\3.\\4" METAFORCE_VERSION_STRING "${METAFORCE_WC_DESCRIBE}")
  string(REGEX REPLACE "v([0-9]+)\.([0-9]+)\.([0-9]+).*" "\\1.\\2.\\3" METAFORCE_SHORT_VERSION_STRING "${METAFORCE_WC_DESCRIBE}")
else ()
  set(METAFORCE_WC_DESCRIBE "UNKNOWN-VERSION")
  set(METAFORCE_VERSION_STRING "0.0.0")
endif ()

string(TIMESTAMP CURRENT_YEAR "%Y")

# Add version information to CI environment variables
if(DEFINED ENV{GITHUB_ENV})
  file(APPEND "$ENV{GITHUB_ENV}" "METAFORCE_VERSION=${METAFORCE_WC_DESCRIBE}\n")
endif()
message(STATUS "Metaforce version set to ${METAFORCE_WC_DESCRIBE}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
if(APPLE)
  set(EXTRA_LANGUAGES OBJC)
endif()
project(metaforce LANGUAGES C CXX ASM ${EXTRA_LANGUAGES} VERSION ${METAFORCE_VERSION_STRING})
if (APPLE AND NOT TVOS AND CMAKE_SYSTEM_NAME STREQUAL tvOS)
  # ios.toolchain.cmake hack for SDL
  set(TVOS ON)
  set(IOS OFF)
endif ()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" ATHENA_ARCH)
string(TOLOWER "${CMAKE_HOST_SYSTEM_PROCESSOR}" ATHENA_HOST_ARCH)
string(TOLOWER "${CMAKE_HOST_SYSTEM_NAME}" HOST_PLATFORM_NAME)
string(TOLOWER "${CMAKE_SYSTEM_NAME}" PLATFORM_NAME)
set(ATHENA_EXTENSION "tar.gz")
if (CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
  set(HOST_PLATFORM_NAME macos)
  set(ATHENA_ARCH universal)
  set(ATHENA_HOST_ARCH universal)
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set(HOST_PLATFORM_NAME win32)
  set(ATHENA_EXTENSION ".7z")
endif ()
if (CMAKE_SYSTEM_NAME STREQUAL Darwin)
  set(PLATFORM_NAME macos)
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
  set(PLATFORM_NAME win32)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)

if(APPLE AND NOT CMAKE_OSX_SYSROOT)
  # If the Xcode SDK is lagging behind system version, CMake needs this done first
  execute_process(COMMAND xcrun --sdk macosx --show-sdk-path
                  OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

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

if (CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 OR CMAKE_SYSTEM_PROCESSOR STREQUAL AMD64)
  set(METAFORCE_VECTOR_ISA "sse41" CACHE STRING "Vector ISA to build for (sse2, sse3, sse41, avx, avx2)")
endif ()

if(MSVC)
  if(${METAFORCE_VECTOR_ISA} STREQUAL "avx2")
    add_compile_options(/arch:AVX2)
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "avx")
    add_compile_options(/arch:AVX)
    add_compile_definitions(__SSE4_1__=1)
    message(STATUS "Building with AVX Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse41")
    add_compile_definitions(__SSE4_1__=1)
    # clang-cl 10 requires -msse4.1, may be fixed in newer versions?
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL Clang)
      add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-msse4.1>)
    endif()
    message(STATUS "Building with SSE4.1 Vector ISA")
  else()
    message(STATUS "Building with SSE2 Vector ISA")
  endif()

  if(${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(VS_OPTIONS "/MP")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT metaforce)
  endif()

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
  if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
  if(${METAFORCE_VECTOR_ISA} STREQUAL "native")
    add_compile_options(-march=native)
    message(STATUS "Building with native ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "avx2")
    add_compile_options(-mavx2)
    message(STATUS "Building with AVX2 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "avx")
    add_compile_options(-mavx)
    message(STATUS "Building with AVX Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse41")
    add_compile_options(-msse4.1)
    message(STATUS "Building with SSE4.1 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse3")
    add_compile_options(-msse3)
    message(STATUS "Building with SSE3 Vector ISA")
  elseif(${METAFORCE_VECTOR_ISA} STREQUAL "sse2")
    add_compile_options(-msse2)
    message(STATUS "Building with SSE2 Vector ISA")
  else()
    message(STATUS "Building with x87 Vector ISA")
  endif()
  endif()

  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-fno-plt HAS_NO_PLT)
  if (HAS_NO_PLT)
    add_compile_options(-fno-plt)
  endif()
  check_cxx_compiler_flag(-fno-asynchronous-unwind-tables HAS_NO_ASYNC_UNWIND_TABLES)
  if (HAS_NO_ASYNC_UNWIND_TABLES AND ${CMAKE_BUILD_TYPE} STREQUAL Release)
    # Binary size reduction
    add_compile_options(-fno-asynchronous-unwind-tables)
  endif()

  if (METAFORCE_ASAN)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++> -fsanitize=address
        -fsanitize-address-use-after-scope)
    add_link_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++> -fsanitize=address
        -fsanitize-address-use-after-scope)
  elseif(METAFORCE_MSAN)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++> -fsanitize=memory
                        -fsanitize-memory-track-origins -fsanitize-recover=all)
  endif()
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
                      $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
                      -Wall -Wno-multichar
                      -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable
                      -Wno-unused-function -Wno-sign-compare -Wno-unknown-pragmas)
  # doesn't work with generator expression in add_compile_options?
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    add_compile_options(-Wno-unknown-warning-option -Wno-unused-private-field)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wno-lto-type-mismatch -Wno-maybe-uninitialized)
  endif()

  if(APPLE)
    add_compile_options(-Wno-error=deprecated-declarations
                        $<$<CONFIG:Release>:-flto=thin>)
  endif()

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug OR ${CMAKE_BUILD_TYPE} STREQUAL RelWithDebInfo)
      # This is required to summarize std::string
      add_compile_options(-fno-limit-debug-info -fno-omit-frame-pointer)
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
  # FIXME GCC 11.1 -flto is completely broken
  if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL 11.1.0)
    message(NOTICE "Working around GCC 11.1 bug; disabling LTO")
    set(USE_LTO OFF)
  endif()
else()
  option(USE_LD_LLD "Link with LLD" OFF)
  option(USE_LD_GOLD "Link with GNU Gold" OFF)
  option(USE_LTO "Enable LTO" OFF)
endif()
if(USE_LD_LLD)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=lld -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
  if("${LD_VERSION}" MATCHES "LLD")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -Wl,--build-id=uuid")
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
    if (USE_SPLIT_DWARF)
      add_compile_options(-gsplit-dwarf -Wl,--gdb-index)
      add_link_options(-gsplit-dwarf -Wl,--gdb-index)
      message(STATUS "GNU gold linker enabled with split DWARF.")
    elseif (USE_LTO)
      add_compile_options(-flto)
      add_link_options(-flto)
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

find_package(ZLIB REQUIRED)
set(ZLIB_LIBRARIES ZLIB::ZLIB CACHE STRING "zlib libraries" FORCE)

include(ExternalProject)
ExternalProject_Add(bintoc
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/bintoc"
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install)
include(${CMAKE_CURRENT_LIST_DIR}/bintoc/bintocHelpers.cmake)

add_subdirectory(extern)
add_subdirectory(imgui)

if(NOT TARGET atdna)
  # Import native atdna if cross-compiling
  find_package(atdna REQUIRED)
  if(NOT TARGET atdna)
    message(FATAL_ERROR "atdna required for building Metaforce; please verify LLVM installation")
  endif()
endif()

add_subdirectory(NESEmulator EXCLUDE_FROM_ALL)
add_subdirectory(aurora)
add_subdirectory(Runtime)
add_subdirectory(gbalink EXCLUDE_FROM_ALL)

configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_BINARY_DIR}/version.h)

# Packaging logic
list(APPEND BINARY_TARGETS metaforce)
set(EXTRA_TARGETS "")
if (TARGET crashpad_handler)
  list(APPEND EXTRA_TARGETS crashpad_handler)
endif ()
set(BIN_PREFIX "${CMAKE_INSTALL_PREFIX}")
install(TARGETS ${BINARY_TARGETS} ${EXTRA_TARGETS} DESTINATION ${BIN_PREFIX})
if (CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
  foreach (target IN LISTS BINARY_TARGETS)
    if (WIN32)
      install(FILES $<TARGET_PDB_FILE:${target}> DESTINATION ${BIN_PREFIX} OPTIONAL)
    elseif (APPLE)
      install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND rm -fr \"$<TARGET_FILE_NAME:${target}>.dSYM\")")
      install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND dsymutil \"$<TARGET_FILE:${target}>\")")
      install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND strip -S \"$<TARGET_FILE:${target}>\")")
      install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND mv \"$<TARGET_FILE:${target}>.dSYM\" .)")
    elseif (UNIX)
      install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND objcopy --only-keep-debug ${target} ${target}.dbg)")
      install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND objcopy --strip-debug --add-gnu-debuglink=${target}.dbg ${target})")
    endif ()
  endforeach ()
  set(DEBUG_FILES_LIST ${BINARY_TARGETS})
  if (WIN32)
    list(TRANSFORM DEBUG_FILES_LIST APPEND ".pdb")
    list(JOIN DEBUG_FILES_LIST " " DEBUG_FILES)
    install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND 7z a -t7z \"${CMAKE_INSTALL_PREFIX}/debug.7z\" ${DEBUG_FILES})")
  elseif (APPLE)
    list(TRANSFORM DEBUG_FILES_LIST APPEND ".dSYM")
    list(JOIN DEBUG_FILES_LIST " " DEBUG_FILES)
    install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND tar acfv \"${CMAKE_INSTALL_PREFIX}/debug.tar.xz\" ${DEBUG_FILES})")
  elseif (UNIX)
    list(TRANSFORM DEBUG_FILES_LIST APPEND ".dbg")
    list(JOIN DEBUG_FILES_LIST " " DEBUG_FILES)
    install(CODE "execute_process(WORKING_DIRECTORY \"${BIN_PREFIX}\" COMMAND tar -I \"xz -9 -T0\" -cvf \"${CMAKE_INSTALL_PREFIX}/debug.tar.xz\" ${DEBUG_FILES})")
  endif ()
endif ()
