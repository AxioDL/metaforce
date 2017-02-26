cmake_minimum_required(VERSION 3.0)
project(visigen)

if(MSVC)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
  -std=c++14 -Wno-multichar -fno-exceptions -Wno-narrowing -Wno-nullability-completeness -Werror=return-type")
endif()

if(APPLE)
  set(PLAT_SRCS MainMac.mm)
  set_source_files_properties(MainMac.mm PROPERTIES COMPILE_FLAGS -fobjc-arc)
elseif(WIN32)
  set(PLAT_SRCS MainWin.cpp)
else()
  set(PLAT_SRCS MainXlib.cpp)
endif()

add_executable(visigen ${PLAT_SRCS}
               VISIRenderer.cpp VISIRenderer.hpp
               VISIBuilder.cpp VISIBuilder.hpp)
target_link_libraries(visigen logvisor athena-core zeus glew xxhash ${BOO_SYS_LIBS})

add_custom_command(TARGET visigen POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:visigen> $<TARGET_FILE_DIR:urde>)
