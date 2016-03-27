add_subdirectory(Backend)
add_subdirectory(Database)
add_subdirectory(Frontend)
add_subdirectory(Runtime)

if(WIN32)
list(APPEND PLAT_SRCS winsupport.cpp ../include/hecl/winsupport.hpp)
endif()

atdna(atdna_HMDLMeta.cpp ../include/hecl/HMDLMeta.hpp)
atdna(atdna_Frontend.cpp ../include/hecl/Frontend.hpp)
atdna(atdna_Runtime.cpp ../include/hecl/Runtime.hpp)
atdna(atdna_CVar.cpp ../include/hecl/CVar.hpp)

add_library(hecl-common
            hecl.cpp
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
            ../include/hecl/Frontend.hpp
            ../include/hecl/Database.hpp
            ../include/hecl/Runtime.hpp
            ../include/hecl/ClientProcess.hpp
            ClientProcess.cpp
            atdna_HMDLMeta.cpp
            atdna_Frontend.cpp
            atdna_Runtime.cpp
            atdna_CVar.cpp
            ${PLAT_SRCS})

