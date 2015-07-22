add_subdirectory(Backend)
add_subdirectory(Database)
add_subdirectory(Frontend)
add_subdirectory(Runtime)

if(WIN32)
list(APPEND PLAT_SRCS winsupport.c ../include/HECL/winsupport.h)
endif()

add_library(HECLCommon
            HECL.cpp
            ProjectPath.cpp
            WideStringConvert.cpp
            ../include/HECL/HECL.hpp
            ../include/HECL/Backend.hpp
            ../include/HECL/Frontend.hpp
            ../include/HECL/Database.hpp
            ../include/HECL/Runtime.hpp
            ${PLAT_SRCS})

