set(AUTOMAPPER_SOURCES
    CMapWorldInfo.hpp CMapWorldInfo.cpp
    CMapWorld.hpp CMapWorld.cpp
    CMapArea.hpp CMapArea.cpp
    CMappableObject.hpp CMappableObject.cpp
    CAutoMapper.hpp CAutoMapper.cpp
    ITweakAutoMapper.hpp)

runtime_add_list(AutoMapper AUTOMAPPER_SOURCES)
