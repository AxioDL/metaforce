macro(hecl_add_list rel_path a_list)
  unset(tmp_list)
  foreach(path IN LISTS ${a_list})
    list(APPEND tmp_list "${rel_path}/${path}")
  endforeach(path)
  set(${a_list} "${tmp_list}" PARENT_SCOPE)
endmacro(hecl_add_list)

add_subdirectory(Blender)
add_subdirectory(Backend)
add_subdirectory(Frontend)
add_subdirectory(Runtime)

if(WIN32)
list(APPEND PLAT_SRCS winsupport.cpp ../include/hecl/winsupport.hpp)
endif()

atdna(atdna_HMDLMeta.cpp ../include/hecl/HMDLMeta.hpp)
atdna(atdna_Frontend.cpp ../include/hecl/Frontend.hpp)
atdna(atdna_Runtime.cpp ../include/hecl/Runtime.hpp)
atdna(atdna_CVar.cpp ../include/hecl/CVar.hpp)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
  add_definitions(-DHECL_MULTIPROCESSOR)
endif()

add_library(hecl-common
            ${BLENDER_SOURCES}
            ${BACKEND_SOURCES}
            ${FRONTEND_SOURCES}
            ${RUNTIME_SOURCES}
            hecl.cpp
            Project.cpp
            ProjectPath.cpp
            WideStringConvert.cpp
            HumanizeNumber.cpp
            CVar.cpp
            CVarManager.cpp
            ../include/hecl/CVar.hpp
            ../include/hecl/CVarManager.hpp
            ../include/hecl/hecl.hpp
            ../include/hecl/HMDLMeta.hpp
            ../include/hecl/Backend/Backend.hpp
            ../include/hecl/Backend/GX.hpp
            ../include/hecl/Backend/ProgrammableCommon.hpp
            ../include/hecl/Backend/GLSL.hpp
            ../include/hecl/Backend/HLSL.hpp
            ../include/hecl/Backend/Metal.hpp
            ../include/hecl/Blender/BlenderConnection.hpp
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
            SteamFinder.cpp
            ClientProcess.cpp
            atdna_HMDLMeta.cpp
            atdna_Frontend.cpp
            atdna_Runtime.cpp
            atdna_CVar.cpp
            ${PLAT_SRCS})

if(WINDOWS_STORE)
    set_property(TARGET hecl-common PROPERTY VS_WINRT_COMPONENT TRUE)
endif()
