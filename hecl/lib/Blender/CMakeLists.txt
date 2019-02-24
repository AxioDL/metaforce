set(BLENDER_SOURCES
    Connection.cpp
    SDNARead.cpp
    HMDL.cpp)

if(UNIX)
  list(APPEND BLENDER_SOURCES closefrom.c)
endif()

hecl_add_list(Blender BLENDER_SOURCES)
