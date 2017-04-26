include_directories(. ..)
add_subdirectory(World)

if(APPLE)
  set(MP1_PLAT_SOURCES
      CFrontEndUITouchBarMac.mm
      CSaveGameScreenTouchBarMac.mm)
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
    CSaveGameScreen.hpp CSaveGameScreen.cpp
    CSaveGameScreenTouchBar.hpp CSaveGameScreenTouchBar.cpp
    CMemoryCardDriver.hpp CMemoryCardDriver.cpp
    CQuitGameScreen.hpp CQuitGameScreen.cpp
    CMessageScreen.hpp CMessageScreen.cpp
    CCredits.hpp CCredits.cpp
    CStateSetterFlow.hpp CStateSetterFlow.cpp
    CAudioStateWin.hpp CAudioStateWin.cpp
    CInGameGuiManager.hpp CInGameGuiManager.cpp
    CInGameGuiManagerCommon.hpp
    CSamusFaceReflection.hpp CSamusFaceReflection.cpp
    CPlayerVisor.hpp CPlayerVisor.cpp
    CSamusHud.hpp CSamusHud.cpp
    CPauseScreenBlur.hpp CPauseScreenBlur.cpp
    CPauseScreen.hpp CPauseScreen.cpp
    CPauseScreenBase.hpp CPauseScreenBase.cpp
    CFaceplateDecoration.hpp CFaceplateDecoration.cpp
    CInventoryScreen.hpp CInventoryScreen.cpp
    CLogBookScreen.hpp CLogBookScreen.cpp
    COptionsScreen.hpp COptionsScreen.cpp
    CSamusDoll.hpp CSamusDoll.cpp
    MP1.hpp MP1.cpp
    ${MP1_PLAT_SOURCES}
    ${MP1_WORLD_SOURCES})

runtime_add_list(MP1 MP1_SOURCES)
