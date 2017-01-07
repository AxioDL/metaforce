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
#include "AutoMapper/ITweakAutoMapper.hpp"
#include "GuiSys/ITweakGui.hpp"

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

extern DataSpec::ITweakGame*   g_tweakGame;
extern DataSpec::ITweakPlayer* g_tweakPlayer;
extern DataSpec::ITweakPlayerControl* g_tweakPlayerControl;
extern DataSpec::ITweakPlayerControl* g_tweakPlayerControlAlt;
extern DataSpec::ITweakPlayerControl* g_currentPlayerControl;
extern DataSpec::ITweakPlayerGun* g_tweakPlayerGun;
extern DataSpec::ITweakGunRes* g_tweakGunRes;
extern DataSpec::ITweakPlayerRes* g_tweakPlayerRes;
extern ITweakAutoMapper* g_tweakAutoMapper;
extern DataSpec::ITweakGui* g_tweakGui;
extern DataSpec::ITweakSlideShow* g_tweakSlideShow;

}

#endif // __URDE_GAMEGLOBALOBJECTS_HPP__
