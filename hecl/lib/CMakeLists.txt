macro(hecl_add_list rel_path a_list)
  unset(tmp_list)
  foreach(path IN LISTS ${a_list})
    list(APPEND tmp_list "${rel_path}/${path}")
  endforeach(path)
  set(${a_list} "${tmp_list}" PARENT_SCOPE)
endmacro(hecl_add_list)

include_directories(../extern/boo/glslang ../extern/boo)

add_subdirectory(Blender)
add_subdirectory(Backend)
add_subdirectory(Frontend)
add_subdirectory(Runtime)

if(WIN32)
list(APPEND PLAT_SRCS winsupport.cpp ../include/hecl/winsupport.hpp)
endif()

atdna(atdna_HMDLMeta.cpp ../include/hecl/HMDLMeta.hpp)
atdna(atdna_Frontend.cpp ../include/hecl/Frontend.hpp)
atdna(atdna_CVar.cpp ../include/hecl/CVar.hpp)
atdna(atdna_SDNARead.cpp ../include/hecl/Blender/SDNARead.hpp)

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
    ../include/hecl/HMDLMeta.hpp
    ../include/hecl/Backend/Backend.hpp
    ../include/hecl/Backend/GX.hpp
    ../include/hecl/Backend/ProgrammableCommon.hpp
    ../include/hecl/Backend/GLSL.hpp
    ../include/hecl/Backend/HLSL.hpp
    ../include/hecl/Backend/Metal.hpp
    ../include/hecl/Blender/Connection.hpp
    ../include/hecl/Blender/SDNARead.hpp
    ../include/hecl/Blender/Token.hpp
    ../include/hecl/SteamFinder.hpp
    ../include/hecl/Frontend.hpp
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
    Pipeline.cpp
    atdna_HMDLMeta.cpp
    atdna_Frontend.cpp
    atdna_CVar.cpp)

add_library(hecl-full
            ${BACKEND_SOURCES}
            ${FRONTEND_SOURCES}
            ${RUNTIME_SOURCES}
            ${BLENDER_SOURCES}
            atdna_SDNARead.cpp
            ${COMMON_SOURCES}
            ${HECL_HEADERS}
            ${PLAT_SRCS})
add_library(hecl-light
            ${RUNTIME_SOURCES}
            ${COMMON_SOURCES}
            ${HECL_HEADERS}
            ${PLAT_SRCS})
add_library(hecl-compilers Compilers.cpp WideStringConvert.cpp)

add_dependencies(hecl-full ${HECL_APPLICATION_REPS_TARGETS_LIST})
add_dependencies(hecl-light ${HECL_APPLICATION_REPS_TARGETS_LIST})

if(COMMAND add_sanitizers)
  add_sanitizers(hecl-full)
  add_sanitizers(hecl-light)
  add_sanitizers(hecl-compilers)
endif()

if(WINDOWS_STORE)
  set_property(TARGET hecl-full PROPERTY VS_WINRT_COMPONENT TRUE)
endif()
