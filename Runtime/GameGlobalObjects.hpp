#pragma once

#define USE_DOWNCAST_TWEAKS 1

#if USE_DOWNCAST_TWEAKS
#include "../DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakPlayerControl.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakPlayerGun.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakGunRes.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakPlayerRes.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakGui.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakSlideShow.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakGui.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakTargeting.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakAutoMapper.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakParticle.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakBall.hpp"
#include "../DataSpec/DNAMP1/Tweaks/CTweakGuiColors.hpp"
#else
#include "../DataSpec/DNACommon/Tweaks/ITweakGame.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakPlayer.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakPlayerControl.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakGunRes.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakPlayerRes.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakGui.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakSlideShow.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakGui.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakTargeting.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakAutoMapper.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakParticle.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakBall.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakGuiColors.hpp"
#endif

#include "Runtime/CTextureCache.hpp"

namespace urde {
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
extern class CBooRenderer* g_Renderer;
extern class CStringTable* g_MainStringTable;
extern class CTextureCache* g_TextureCache;
extern class CInputGenerator* g_InputGenerator;
extern class CStateManager* g_StateManager;

#if USE_DOWNCAST_TWEAKS
using ITweakGame = DataSpec::DNAMP1::CTweakGame;
using ITweakPlayer = DataSpec::DNAMP1::CTweakPlayer;
using ITweakPlayerControl = DataSpec::DNAMP1::CTweakPlayerControl;
using ITweakPlayerGun = DataSpec::DNAMP1::CTweakPlayerGun;
using ITweakGunRes = DataSpec::DNAMP1::CTweakGunRes;
using ITweakAutoMapper = DataSpec::DNAMP1::CTweakAutoMapper;
using ITweakGui = DataSpec::DNAMP1::CTweakGui;
using ITweakSlideShow = DataSpec::DNAMP1::CTweakSlideShow;
using ITweakParticle = DataSpec::DNAMP1::CTweakParticle;
using ITweakBall = DataSpec::DNAMP1::CTweakBall;
using ITweakGuiColors = DataSpec::DNAMP1::CTweakGuiColors;
#else
using ITweakGame = DataSpec::ITweakGame;
using ITweakPlayer = DataSpec::ITweakPlayer;
using ITweakPlayerControl = DataSpec::ITweakPlayerControl;
using ITweakPlayerGun = DataSpec::ITweakPlayerGun;
using ITweakGunRes = DataSpec::ITweakGunRes;
using ITweakAutoMapper = DataSpec::ITweakAutoMapper;
using ITweakGui = DataSpec::ITweakGui;
using ITweakSlideShow = DataSpec::ITweakSlideShow;
using ITweakParticle = DataSpec::ITweakParticle;
using ITweakBall = DataSpec::ITweakBall;
using ITweakGuiColors = DataSpec::ITweakGuiColors;
#endif
using ITweakPlayerRes = DataSpec::ITweakPlayerRes;
using ITweakTargeting = DataSpec::ITweakTargeting;

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

} // namespace urde
