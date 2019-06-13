macro(hecl_add_list rel_path a_list)
  unset(tmp_list)
  foreach(path IN LISTS ${a_list})
    list(APPEND tmp_list "${rel_path}/${path}")
  endforeach(path)
  set(${a_list} "${tmp_list}" PARENT_SCOPE)
endmacro(hecl_add_list)

add_subdirectory(Blender)
add_subdirectory(Runtime)

if(WIN32)
list(APPEND PLAT_SRCS ../include/hecl/winsupport.hpp)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  add_definitions(-DHECL_MULTIPROCESSOR)
endif()

set(HECL_HEADERS
    ../include/hecl/CVar.hpp
    ../include/hecl/CVarManager.hpp
    ../include/hecl/Console.hpp
    ../include/hecl/CVarCommons.hpp
    ../include/hecl/hecl.hpp
    ../include/hecl/MultiProgressPrinter.hpp
    ../include/hecl/FourCC.hpp
    ../include/hecl/TypedVariant.hpp
    ../include/hecl/HMDLMeta.hpp
    ../include/hecl/Backend.hpp
    ../include/hecl/Blender/Connection.hpp
    ../include/hecl/Blender/SDNARead.hpp
    ../include/hecl/Blender/Token.hpp
    ../include/hecl/SteamFinder.hpp
    ../include/hecl/Database.hpp
    ../include/hecl/Runtime.hpp
    ../include/hecl/ClientProcess.hpp
    ../include/hecl/SystemChar.hpp
    ../include/hecl/BitVector.hpp
    ../include/hecl/MathExtras.hpp
    ../include/hecl/UniformBufferPool.hpp
    ../include/hecl/VertexBufferPool.hpp
    ../include/hecl/PipelineBase.hpp
    ../include/hecl/Pipeline.hpp
    ../include/hecl/Compilers.hpp)
set(COMMON_SOURCES
    hecl.cpp
    MultiProgressPrinter.cpp
    Project.cpp
    ProjectPath.cpp
    HumanizeNumber.cpp
    CVar.cpp
    CVarManager.cpp
    Console.cpp
    ClientProcess.cpp
    SteamFinder.cpp
    WideStringConvert.cpp
    Compilers.cpp
    Pipeline.cpp)

if(UNIX)
  list(APPEND PLAT_SRCS closefrom.c)
endif()

add_library(hecl-full
            ${FRONTEND_SOURCES}
            ${RUNTIME_SOURCES}
            ${BLENDER_SOURCES}
            ${COMMON_SOURCES}
            ${HECL_HEADERS}
            ${PLAT_SRCS})
target_include_directories(hecl-full PUBLIC ../include)
target_link_libraries(hecl-full PUBLIC ${HECL_APPLICATION_REPS_TARGETS_LIST}
                      hecl-blender-addon boo athena-core logvisor)
target_atdna(hecl-full atdna_HMDLMeta_full.cpp ../include/hecl/HMDLMeta.hpp)
target_atdna(hecl-full atdna_CVar_full.cpp ../include/hecl/CVar.hpp)
target_atdna(hecl-full atdna_SDNARead_full.cpp ../include/hecl/Blender/SDNARead.hpp)

add_library(hecl-light
            ${RUNTIME_SOURCES}
            ${COMMON_SOURCES}
            ${HECL_HEADERS}
            ${PLAT_SRCS})
target_include_directories(hecl-light PUBLIC ../include)
target_link_libraries(hecl-light PUBLIC ${HECL_APPLICATION_REPS_TARGETS_LIST} boo athena-core logvisor)
target_atdna(hecl-light atdna_HMDLMeta_light.cpp ../include/hecl/HMDLMeta.hpp)
target_atdna(hecl-light atdna_CVar_light.cpp ../include/hecl/CVar.hpp)

add_library(hecl-compilers Compilers.cpp WideStringConvert.cpp)
target_include_directories(hecl-compilers PUBLIC ../include)
target_link_libraries(hecl-compilers PUBLIC boo athena-core logvisor)

if(COMMAND add_sanitizers)
  add_sanitizers(hecl-full)
  add_sanitizers(hecl-light)
  add_sanitizers(hecl-compilers)
endif()

if(WINDOWS_STORE)
  set_property(TARGET hecl-full PROPERTY VS_WINRT_COMPONENT TRUE)
endif()
