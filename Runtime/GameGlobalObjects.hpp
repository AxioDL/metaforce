#pragma once

#define USE_DOWNCAST_TWEAKS 1

#if USE_DOWNCAST_TWEAKS
#include "Runtime/MP1/Tweaks/CTweakAutoMapper.hpp"
#include "Runtime/MP1/Tweaks/CTweakBall.hpp"
#include "Runtime/MP1/Tweaks/CTweakGame.hpp"
#include "Runtime/MP1/Tweaks/CTweakGui.hpp"
#include "Runtime/MP1/Tweaks/CTweakGui.hpp"
#include "Runtime/MP1/Tweaks/CTweakGuiColors.hpp"
#include "Runtime/MP1/Tweaks/CTweakGunRes.hpp"
#include "Runtime/MP1/Tweaks/CTweakParticle.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayer.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayerControl.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayerGun.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayerRes.hpp"
#include "Runtime/MP1/Tweaks/CTweakSlideShow.hpp"
#include "Runtime/MP1/Tweaks/CTweakTargeting.hpp"
#include "Runtime/MP1/Tweaks/CTweakGuiColors.hpp"
#include "Runtime/MP1/Tweaks/CTweakTargeting.hpp"
#else
#include "Runtime/Tweaks/ITweakAutoMapper.hpp"
#include "Runtime/Tweaks/ITweakBall.hpp"
#include "Runtime/Tweaks/ITweakGame.hpp"
#include "Runtime/Tweaks/ITweakGui.hpp"
#include "Runtime/Tweaks/ITweakGui.hpp"
#include "Runtime/Tweaks/ITweakGuiColors.hpp"
#include "Runtime/Tweaks/ITweakGunRes.hpp"
#include "Runtime/Tweaks/ITweakParticle.hpp"
#include "Runtime/Tweaks/ITweakPlayer.hpp"
#include "Runtime/Tweaks/ITweakPlayerControl.hpp"
#include "Runtime/Tweaks/ITweakPlayerGun.hpp"
#include "Runtime/Tweaks/ITweakPlayerRes.hpp"
#include "Runtime/Tweaks/ITweakSlideShow.hpp"
#include "Runtime/Tweaks/ITweakTargeting.hpp"
#endif

#include "Runtime/CTextureCache.hpp"

namespace metaforce {
extern class IMain* g_Main;
namespace MP1 {
extern class CGameArchitectureSupport* g_archSupport;
}
extern class CMemoryCardSys* g_MemoryCardSys;
extern class IFactory* g_ResFactory;
extern class CSimplePool* g_SimplePool;
extern class CCharacterFactoryBuilder* g_CharFactoryBuilder;
extern class CAiFuncMap* g_AiFuncMap;
extern class CGameState* g_GameState;
extern class CInGameTweakManagerBase* g_TweakManager;
extern class CCubeRenderer* g_Renderer;
extern class CStringTable* g_MainStringTable;
extern class CTextureCache* g_TextureCache;
extern class CInputGenerator* g_InputGenerator;
extern class IController* g_Controller;
extern class CStateManager* g_StateManager;

#if USE_DOWNCAST_TWEAKS
using ITweakGame = metaforce::MP1::CTweakGame;
using ITweakPlayer = metaforce::MP1::CTweakPlayer;
using ITweakPlayerControl = metaforce::MP1::CTweakPlayerControl;
using ITweakPlayerGun = metaforce::MP1::CTweakPlayerGun;
using ITweakGunRes = metaforce::MP1::CTweakGunRes;
using ITweakAutoMapper = metaforce::MP1::CTweakAutoMapper;
using ITweakGui = metaforce::MP1::CTweakGui;
using ITweakSlideShow = metaforce::MP1::CTweakSlideShow;
using ITweakParticle = metaforce::MP1::CTweakParticle;
using ITweakBall = metaforce::MP1::CTweakBall;
using ITweakGuiColors = metaforce::MP1::CTweakGuiColors;
using ITweakPlayerRes = metaforce::MP1::CTweakPlayerRes;
using ITweakTargeting = metaforce::MP1::CTweakTargeting;
#else
using ITweakGame = metaforce::Tweaks::ITweakGame;
using ITweakPlayer = metaforce::Tweaks::ITweakPlayer;
using ITweakPlayerControl = metaforce::Tweaks::ITweakPlayerControl;
using ITweakPlayerGun = metaforce::Tweaks::ITweakPlayerGun;
using ITweakGunRes = metaforce::Tweaks::ITweakGunRes;
using ITweakAutoMapper = metaforce::Tweaks::ITweakAutoMapper;
using ITweakGui = metaforce::Tweaks::ITweakGui;
using ITweakSlideShow = metaforce::Tweaks::ITweakSlideShow;
using ITweakParticle = metaforce::Tweaks::ITweakParticle;
using ITweakBall = metaforce::Tweaks::ITweakBall;
using ITweakGuiColors = metaforce::Tweaks::ITweakGuiColors;
using ITweakPlayerRes = metaforce::Tweaks::ITweakPlayerRes;
using ITweakTargeting = metaforce::Tweaks::ITweakTargeting;
#endif

extern ITweakGame* g_tweakGame;
extern ITweakPlayer* g_tweakPlayer;
extern ITweakPlayerControl* g_tweakPlayerControl;
extern ITweakPlayerControl* g_tweakPlayerControlAlt;
extern ITweakPlayerControl* g_currentPlayerControl;
extern ITweakPlayerGun* g_tweakPlayerGun;
extern ITweakGunRes* g_tweakGunRes;
extern ITweakPlayerRes* g_tweakPlayerRes;
extern ITweakTargeting* g_tweakTargeting;
extern ITweakAutoMapper* g_tweakAutoMapper;
extern ITweakGui* g_tweakGui;
extern ITweakSlideShow* g_tweakSlideShow;
extern ITweakParticle* g_tweakParticle;
extern ITweakBall* g_tweakBall;
extern ITweakGuiColors* g_tweakGuiColors;

} // namespace metaforce
