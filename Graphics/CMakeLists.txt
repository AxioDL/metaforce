if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CARGO_CMD cargo build --verbose)
  set(TARGET_DIR "debug")
else ()
  set(CARGO_CMD cargo build --release --verbose)
  set(TARGET_DIR "release")
endif ()

if(USE_LTO)
  set(RUST_FLAGS "-Clinker-plugin-lto" "-Clinker=clang" "-Clink-arg=-fuse-ld=lld")
endif()

set(AURORA_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libaurora.a")

set(AURORA_CXX "${CMAKE_CURRENT_BINARY_DIR}/aurora.cpp")
set(AURORA_SHADERS_CXX "${CMAKE_CURRENT_BINARY_DIR}/aurora_shaders.cpp")
set(AURORA_IMGUI_CXX "${CMAKE_CURRENT_BINARY_DIR}/aurora_imgui.cpp")
add_library(aurora STATIC ${AURORA_CXX} ${AURORA_SHADERS_CXX})
target_include_directories(aurora PRIVATE ../Runtime)
target_include_directories(aurora PUBLIC include ${CMAKE_CURRENT_BINARY_DIR})
add_custom_command(
    OUTPUT ${AURORA_CXX} ${AURORA_SHADERS_CXX} ${AURORA_IMGUI_CXX} ${AURORA_LIB} #__no_exist
    COMMAND CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} RUSTFLAGS="${RUST_FLAGS}" ${CARGO_CMD}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/aurora/src/cxxbridge.rs.cc ${AURORA_CXX}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/aurora/src/cxxbridge.rs.h ${CMAKE_CURRENT_BINARY_DIR}/aurora.h
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/aurora/src/shaders/cxxbridge.rs.cc ${AURORA_SHADERS_CXX}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/aurora/src/shaders/cxxbridge.rs.h ${CMAKE_CURRENT_BINARY_DIR}/aurora_shaders.h
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/aurora/src/cxxbridge.rs.cc ${AURORA_IMGUI_CXX}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/aurora/src/cxxbridge.rs.h ${CMAKE_CURRENT_BINARY_DIR}/aurora_imgui.h
    DEPENDS src/cxxbridge.rs
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_custom_target(
    always_run ALL
    DEPENDS __no_exist
)

target_link_libraries(aurora pthread dl ${AURORA_LIB} zeus)

add_test(NAME aurora_test
    COMMAND cargo test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
