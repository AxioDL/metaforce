if(NOT WINDOWS_STORE)
  list(APPEND PLAT_SRCS GLSL.cpp HLSL.cpp Metal.cpp)
else()
  list(APPEND PLAT_SRCS HLSL.cpp)
endif()

set(BACKEND_SOURCES
    GX.cpp
    ProgrammableCommon.cpp
    ${PLAT_SRCS})

hecl_add_list(Backend BACKEND_SOURCES)
