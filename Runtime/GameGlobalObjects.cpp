#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce {
namespace MP1 {
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
class CCubeRenderer* g_Renderer = nullptr;
class CStringTable* g_MainStringTable = nullptr;
class CTextureCache* g_TextureCache = nullptr;
class CInputGenerator* g_InputGenerator = nullptr;
class IController* g_Controller = nullptr;
class CStateManager* g_StateManager = nullptr;

ITweakGame* g_tweakGame = nullptr;
ITweakPlayer* g_tweakPlayer = nullptr;
ITweakPlayerControl* g_tweakPlayerControl = nullptr;
ITweakPlayerControl* g_tweakPlayerControlAlt = nullptr;
ITweakPlayerControl* g_currentPlayerControl = nullptr;
ITweakPlayerGun* g_tweakPlayerGun = nullptr;
ITweakGunRes* g_tweakGunRes = nullptr;
ITweakPlayerRes* g_tweakPlayerRes = nullptr;
ITweakTargeting* g_tweakTargeting = nullptr;
ITweakAutoMapper* g_tweakAutoMapper = nullptr;
ITweakGui* g_tweakGui = nullptr;
ITweakSlideShow* g_tweakSlideShow = nullptr;
ITweakParticle* g_tweakParticle = nullptr;
ITweakBall* g_tweakBall = nullptr;
ITweakGuiColors* g_tweakGuiColors = nullptr;

} // namespace metaforce
