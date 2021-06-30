set(BLENDER_SOURCES
        Connection.cpp
        MeshOptimizer.hpp
        MeshOptimizer.cpp
        SDNARead.cpp
        HMDL.cpp)

hecl_add_list(Blender BLENDER_SOURCES)

set(FIND_BLENDER_SOURCES
        FindBlender.cpp)

hecl_add_list(Blender FIND_BLENDER_SOURCES)