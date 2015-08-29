#ifndef __RETRO_GAMEGLOBALOBJECTS_HPP__
#define __RETRO_GAMEGLOBALOBJECTS_HPP__

namespace Retro
{

extern class CMemoryCardSys* g_MemoryCardSys;
extern class CResFactory* g_ResFactory;
extern class CSimplePool* g_SimplePool;
extern class CCharacterFactoryBuilder* g_CharFactoryBuilder;
extern class CAiFuncMap* g_AiFuncMap;
extern class CGameState* g_GameState;
extern class CInGameTweakManagerBase* g_TweakManager;
extern class CBooRenderer* g_Renderer;

extern class ITweakPlayer* g_tweakPlayer;
extern class ITweakPlayerControl* g_tweakPlayerControl;

}

#endif // __RETRO_GAMEGLOBALOBJECTS_HPP__
