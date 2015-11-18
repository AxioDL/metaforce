if(WIN32)
  set(PLAT_SRCS HLSL.cpp)
endif()
if(APPLE)
  set(PLAT_SRCS Metal.cpp)
endif()

add_library(HECLBackend
            GX.cpp
            ProgrammableCommon.cpp
            GLSL.cpp
            ${PLAT_SRCS})
