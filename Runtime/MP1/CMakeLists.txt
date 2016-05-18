include_directories(. ..)
set(MP1_SOURCES
    CTweaks.hpp CTweaks.cpp
    CInGameTweakManager.hpp CInGameTweakManager.cpp
    CMainFlow.hpp CMainFlow.cpp
    CMFGame.hpp CMFGame.cpp
    CPlayMovie.hpp CPlayMovie.cpp
    CFrontEndUI.hpp CFrontEndUI.cpp
    CNewIntroBoss.hpp CNewIntroBoss.cpp
    CBeetle.hpp CBeetle.cpp
    CWarWasp.hpp CWarWasp.cpp
    MP1.hpp MP1.cpp)

runtime_add_list(MP1 MP1_SOURCES)
