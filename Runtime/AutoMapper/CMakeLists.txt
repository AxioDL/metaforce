set(AUTOMAPPER_SOURCES
    CMapUniverse.hpp CMapUniverse.cpp
    CMapWorldInfo.hpp CMapWorldInfo.cpp
    CMapWorld.hpp CMapWorld.cpp
    CMapArea.hpp CMapArea.cpp
    CMappableObject.hpp CMappableObject.cpp
    CAutoMapper.hpp CAutoMapper.cpp)

runtime_add_list(AutoMapper AUTOMAPPER_SOURCES)
