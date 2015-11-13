add_subdirectory(Backend)
add_subdirectory(Database)
add_subdirectory(Frontend)
add_subdirectory(Runtime)

if(WIN32)
list(APPEND PLAT_SRCS winsupport.cpp ../include/HECL/winsupport.hpp)
endif()

atdna(atdna_Frontend.cpp ../include/HECL/Frontend.hpp)
atdna(atdna_Runtime.cpp ../include/HECL/Runtime.hpp)

add_library(HECLCommon
            HECL.cpp
            ProjectPath.cpp
            WideStringConvert.cpp
            ../include/HECL/HECL.hpp
            ../include/HECL/Backend/Backend.hpp
            ../include/HECL/Backend/GX.hpp
            ../include/HECL/Backend/ProgrammableCommon.hpp
            ../include/HECL/Backend/GLSL.hpp
            ../include/HECL/Frontend.hpp
            ../include/HECL/Database.hpp
            ../include/HECL/Runtime.hpp
            atdna_Frontend.cpp
            atdna_Runtime.cpp
            ${PLAT_SRCS})

