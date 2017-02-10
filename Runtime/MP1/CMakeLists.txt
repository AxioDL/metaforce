include_directories(. ..)
add_subdirectory(World)

if(APPLE)
  set(MP1_PLAT_SOURCES CFrontEndUITouchBarMac.mm)
endif()

set(MP1_SOURCES
    CTweaks.hpp CTweaks.cpp
    CInGameTweakManager.hpp CInGameTweakManager.cpp
    CGBASupport.hpp CGBASupport.cpp
    CNESEmulator.hpp CNESEmulator.cpp
    CMainFlow.hpp CMainFlow.cpp
    CMFGame.hpp CMFGame.cpp
    CPlayMovie.hpp CPlayMovie.cpp
    CFrontEndUI.hpp CFrontEndUI.cpp
    CFrontEndUITouchBar.hpp CFrontEndUITouchBar.cpp
    CPreFrontEnd.hpp CPreFrontEnd.cpp
    CSlideShow.hpp CSlideShow.cpp
    CSaveUI.hpp CSaveUI.cpp
    CMemoryCardDriver.hpp CMemoryCardDriver.cpp
    CQuitScreen.hpp CQuitScreen.cpp
    CCredits.hpp CCredits.cpp
    CStateSetterFlow.hpp CStateSetterFlow.cpp
    CAudioStateWin.hpp CAudioStateWin.cpp
    MP1.hpp MP1.cpp
    ${MP1_PLAT_SOURCES}
    ${MP1_WORLD_SOURCES})

runtime_add_list(MP1 MP1_SOURCES)
