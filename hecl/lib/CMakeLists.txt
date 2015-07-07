add_subdirectory(backend)
add_subdirectory(database)
add_subdirectory(frontend)
add_subdirectory(runtime)

add_library(HECL
            HECL.cpp
            ProjectPath.cpp
            WideStringConvert.cpp)

