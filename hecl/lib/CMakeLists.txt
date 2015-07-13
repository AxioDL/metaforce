add_subdirectory(Backend)
add_subdirectory(Database)
add_subdirectory(Frontend)
add_subdirectory(Runtime)

add_library(HECL
            HECL.cpp
            ProjectPath.cpp
            WideStringConvert.cpp
            ../include/HECL/HECL.hpp
            ../include/HECL/Backend.hpp
            ../include/HECL/Frontend.hpp
            ../include/HECL/Database.hpp
            ../include/HECL/Runtime.hpp)

