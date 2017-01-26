include_directories(. ..)
set(MP1_SOURCES
    CTweaks.hpp CTweaks.cpp
    CInGameTweakManager.hpp CInGameTweakManager.cpp
    CGBASupport.hpp CGBASupport.cpp
    CNESEmulator.hpp CNESEmulator.cpp
    CMainFlow.hpp CMainFlow.cpp
    CMFGame.hpp CMFGame.cpp
    CPlayMovie.hpp CPlayMovie.cpp
    CFrontEndUI.hpp CFrontEndUI.cpp
    CPreFrontEnd.hpp CPreFrontEnd.cpp
    CSlideShow.hpp CSlideShow.cpp
    CNewIntroBoss.hpp CNewIntroBoss.cpp
    CBeetle.hpp CBeetle.cpp
    CWarWasp.hpp CWarWasp.cpp
    CSpacePirate.hpp CSpacePirate.cpp
    CBabygoth.hpp CBabygoth.cpp
    CMetroidPrimeRelay.hpp CMetroidPrimeRelay.cpp
    CSaveUI.hpp CSaveUI.cpp
    CMemoryCardDriver.hpp CMemoryCardDriver.cpp
    CQuitScreen.hpp CQuitScreen.cpp
    CCredits.hpp CCredits.cpp
    CStateSetterFlow.hpp CStateSetterFlow.cpp
    CAudioStateWin.hpp CAudioStateWin.cpp
    MP1.hpp MP1.cpp)

runtime_add_list(MP1 MP1_SOURCES)
