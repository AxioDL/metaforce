#ifndef __URDE_GAMEGLOBALOBJECTS_HPP__
#define __URDE_GAMEGLOBALOBJECTS_HPP__
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

namespace urde
{
extern class IMain* g_Main;
namespace MP1
{
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
extern class CInputGenerator* g_InputGenerator;

using ITweakGame = DataSpec::ITweakGame;
using ITweakPlayer = DataSpec::ITweakPlayer;
using ITweakPlayerRes = DataSpec::ITweakPlayerRes;
using ITweakPlayerControl = DataSpec::ITweakPlayerControl;
using ITweakPlayerGun = DataSpec::ITweakPlayerGun;
using ITweakGunRes = DataSpec::ITweakGunRes;
using ITweakTargeting = DataSpec::ITweakTargeting;
using ITweakAutoMapper = DataSpec::ITweakAutoMapper;
using ITweakGui = DataSpec::ITweakGui;
using ITweakSlideShow = DataSpec::ITweakSlideShow;
using ITweakParticle = DataSpec::ITweakParticle;
using ITweakBall = DataSpec::ITweakBall;
using ITweakGuiColors = DataSpec::ITweakGuiColors;

extern ITweakGame*   g_tweakGame;
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

}

#endif // __URDE_GAMEGLOBALOBJECTS_HPP__
