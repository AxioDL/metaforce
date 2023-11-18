add_subdirectory(World)

set(MP1_SOURCES
        Tweaks/CTweakAutoMapper.hpp Tweaks/CTweakAutoMapper.cpp
        Tweaks/CTweakBall.hpp Tweaks/CTweakBall.cpp
        Tweaks/CTweakGame.hpp Tweaks/CTweakGame.cpp
        Tweaks/CTweakGui.hpp Tweaks/CTweakGui.cpp
        Tweaks/CTweakGuiColors.hpp Tweaks/CTweakGuiColors.cpp
        Tweaks/CTweakGunRes.hpp Tweaks/CTweakGunRes.cpp
        Tweaks/CTweakParticle.hpp Tweaks/CTweakParticle.cpp
        Tweaks/CTweakPlayer.hpp Tweaks/CTweakPlayer.cpp
        Tweaks/CTweakPlayerControl.hpp Tweaks/CTweakPlayerControl.cpp
        Tweaks/CTweakPlayerGun.hpp Tweaks/CTweakPlayerGun.cpp
        Tweaks/CTweakPlayerRes.hpp Tweaks/CTweakPlayerRes.cpp
        Tweaks/CTweakSlideShow.hpp Tweaks/CTweakSlideShow.cpp
        Tweaks/CTweakTargeting.hpp Tweaks/CTweakTargeting.cpp
        CTweaks.hpp CTweaks.cpp
        CInGameTweakManager.hpp
        CGBASupport.hpp CGBASupport.cpp
        CMainFlow.hpp CMainFlow.cpp
        CMFGame.hpp CMFGame.cpp
        CPlayMovie.hpp CPlayMovie.cpp
        CFrontEndUI.hpp CFrontEndUI.cpp
        CPreFrontEnd.hpp CPreFrontEnd.cpp
        CSlideShow.hpp CSlideShow.cpp
        CSaveGameScreen.hpp CSaveGameScreen.cpp
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
        CGameCubeDoll.hpp CGameCubeDoll.cpp
        CArtifactDoll.hpp CArtifactDoll.cpp
        CAutoSave.hpp CAutoSave.cpp
        MP1.hpp MP1.cpp
        ${MP1_PLAT_SOURCES}
        ${MP1_WORLD_SOURCES})

runtime_add_list(MP1 MP1_SOURCES)
