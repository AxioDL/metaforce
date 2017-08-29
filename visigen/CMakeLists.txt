cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR) # because of CMAKE_CXX_STANDARD
project(visigen)

set(CMAKE_CXX_STANDARD 14)
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
target_link_libraries(visigen logvisor athena-core athena-libyaml zeus glew xxhash ${ZLIB_LIBRARIES} ${LZO_LIB} ${BOO_SYS_LIBS})
