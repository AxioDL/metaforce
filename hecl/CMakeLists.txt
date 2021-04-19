if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17
project(hecl)

if(MSVC)
  add_compile_options(
    # Disable exceptions
    $<$<COMPILE_LANGUAGE:CXX>:/EHsc>
    /wd4267 /wd4244
  )
  add_compile_definitions(UNICODE=1 _UNICODE=1 _CRT_SECURE_NO_WARNINGS=1)
else()
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  add_compile_options(
    # Disable exceptions
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    -Wno-multichar
  )
endif()
endif()

include(ExternalProject)
ExternalProject_Add(bintoc
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bintoc"
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    INSTALL_COMMAND ${CMAKE_COMMAND} --build . --config Release --target install)
include(bintoc/bintocHelpers.cmake)

if(NOT TARGET atdna)
  # Import native atdna if cross-compiling
  find_package(atdna REQUIRED)
  if(NOT TARGET atdna)
    message(FATAL_ERROR "atdna required for building hecl; please verify LLVM installation")
  endif()
endif()

add_subdirectory(lib)
add_subdirectory(blender)
add_subdirectory(driver)

configure_file(DataSpecRegistry.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/DataSpecRegistry.hpp @ONLY)
target_include_directories(hecl PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

install(DIRECTORY include/hecl DESTINATION include/hecl)
