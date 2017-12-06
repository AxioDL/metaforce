if(WIN32)
  set(PLAT_SRCS HLSL.cpp)
endif()
if(APPLE)
  set(PLAT_SRCS Metal.cpp)
endif()
if(NOT WINDOWS_STORE)
  list(APPEND PLAT_SRCS GLSL.cpp)
endif()

set(BACKEND_SOURCES
    GX.cpp
    ProgrammableCommon.cpp
    ${PLAT_SRCS})

hecl_add_list(Backend BACKEND_SOURCES)
