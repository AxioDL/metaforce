include_directories(. ..)
set(MP1_SOURCES
    CTweaks.hpp CTweaks.cpp
    CInGameTweakManager.hpp CInGameTweakManager.cpp
    Tweaks/CTweakAutoMapper.hpp Tweaks/CTweakAutoMapper.cpp
    Tweaks/CTweakPlayer.hpp Tweaks/CTweakPlayer.cpp
    Tweaks/CTweakGui.hpp Tweaks/CTweakGui.cpp
    CGBASupport.hpp CGBASupport.cpp
    CNESEmulator.hpp CNESEmulator.cpp
    CMainFlow.hpp CMainFlow.cpp
    CMFGame.hpp CMFGame.cpp
    CPlayMovie.hpp CPlayMovie.cpp
    CFrontEndUI.hpp CFrontEndUI.cpp
    CSlideShow.hpp CSlideShow.cpp
    CNewIntroBoss.hpp CNewIntroBoss.cpp
    CBeetle.hpp CBeetle.cpp
    CWarWasp.hpp CWarWasp.cpp
    CSpacePirate.hpp CSpacePirate.cpp
    CBabygoth.hpp CBabygoth.cpp
    CSaveUI.hpp CSaveUI.cpp
    CMemoryCardDriver.hpp CMemoryCardDriver.cpp
    MP1.hpp MP1.cpp)

runtime_add_list(MP1 MP1_SOURCES)
