#include "GameGlobalObjects.hpp"

namespace urde
{
namespace MP1
{
class CGameArchitectureSupport* g_archSupport = nullptr;
}

class IMain* g_Main = nullptr;
class CMemoryCardSys* g_MemoryCardSys = nullptr;
class IFactory* g_ResFactory = nullptr;
class CSimplePool* g_SimplePool = nullptr;
class CCharacterFactoryBuilder* g_CharFactoryBuilder = nullptr;
class CAiFuncMap* g_AiFuncMap = nullptr;
class CGameState* g_GameState = nullptr;
class CInGameTweakManagerBase* g_TweakManager = nullptr;
class CBooRenderer* g_Renderer = nullptr;
class CStringTable* g_MainStringTable = nullptr;

DataSpec::ITweakGame*   g_tweakGame = nullptr;
DataSpec::ITweakPlayer* g_tweakPlayer = nullptr;
DataSpec::ITweakPlayerControl* g_tweakPlayerControl = nullptr;
DataSpec::ITweakPlayerControl* g_tweakPlayerControlAlt = nullptr;
DataSpec::ITweakPlayerControl* g_currentPlayerControl = nullptr;
DataSpec::ITweakPlayerGun* g_tweakPlayerGun = nullptr;
DataSpec::ITweakGunRes* g_tweakGunRes = nullptr;
DataSpec::ITweakPlayerRes* g_tweakPlayerRes = nullptr;
DataSpec::ITweakGui* g_tweakGui = nullptr;
DataSpec::ITweakSlideShow* g_tweakSlideShow = nullptr;

}
