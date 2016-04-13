#ifndef __URDE_GAMEGLOBALOBJECTS_HPP__
#define __URDE_GAMEGLOBALOBJECTS_HPP__
#include "../DataSpec/DNACommon/Tweaks/ITweakGame.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakPlayer.hpp"
#include "../DataSpec/DNACommon/Tweaks/ITweakPlayerControl.hpp"

namespace urde
{

extern class CMemoryCardSys* g_MemoryCardSys;
extern class CResFactory* g_ResFactory;
extern class CSimplePool* g_SimplePool;
extern class CCharacterFactoryBuilder* g_CharFactoryBuilder;
extern class CAiFuncMap* g_AiFuncMap;
extern class CGameState* g_GameState;
extern class CInGameTweakManagerBase* g_TweakManager;
extern class CBooRenderer* g_Renderer;

extern DataSpec::ITweakGame*   g_tweakGame;
extern DataSpec::ITweakPlayer* g_tweakPlayer;
extern DataSpec::ITweakPlayerControl* g_tweakPlayerControl;

}

#endif // __URDE_GAMEGLOBALOBJECTS_HPP__
