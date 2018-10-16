if(NOT WINDOWS_STORE)
cmake_minimum_required(VERSION 3.10 FATAL_ERROR) # because of c++17
project(visigen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}\
  -Wno-multichar -fno-exceptions -Wno-narrowing -Wno-nullability-completeness -Werror=return-type")
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

set_target_properties(visigen PROPERTIES COMPILE_DEFINITIONS URDE_DLPACKAGE="${URDE_DLPACKAGE}")

if(APPLE)
  find_library(OPENGL_LIBRARY OpenGL)
  set(PLAT_LIBS ${OPENGL_LIBRARY})
endif()

target_link_libraries(visigen logvisor athena-core athena-libyaml zeus glew
                      xxhash ${ZLIB_LIBRARIES} ${LZO_LIB} ${BOO_SYS_LIBS} ${PLAT_LIBS})
endif()
