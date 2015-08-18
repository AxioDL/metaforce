#include <memory>
#include "COsContext.hpp"
#include "CBasics.hpp"
#include "CTweaks.hpp"
#include "CMemory.hpp"
#include "CMemoryCardSys.hpp"
#include "CResFactory.hpp"
#include "CSimplePool.hpp"
#include "Character/CAssetFactory.hpp"
#include "CAi.hpp"
#include "CGameState.hpp"
#include "CInGameTweakManager.hpp"
#include "Particle/CElementGen.hpp"
#include "Character/CAnimData.hpp"
#include "Particle/CDecalManager.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "Audio/CAudioSys.hpp"
#include "Input/CInputGenerator.hpp"
#include "GuiSys/CGuiSys.hpp"
#include "CIOWinManager.hpp"
#include "GuiSys/CSplashScreen.hpp"
#include "CMainFlow.hpp"
#include "GuiSys/CConsoleOutputWindow.hpp"
#include "Audio/CAudioStateWin.hpp"
#include "GameGlobalObjects.hpp"

namespace Retro
{
namespace Common
{
CMemoryCardSys* g_MemoryCardSys = nullptr;
CResFactory* g_ResFactory = nullptr;
CSimplePool* g_SimplePool = nullptr;
CCharacterFactoryBuilder* g_CharFactoryBuilder = nullptr;
CAiFuncMap* g_AiFuncMap = nullptr;
CGameState* g_GameState = nullptr;
CInGameTweakManager* g_TweakManager = nullptr;
CBooRenderer* g_Renderer = nullptr;
}

namespace MP1
{

class CGameGlobalObjects
{
    Common::CMemoryCardSys m_memoryCardSys;
    Common::CResFactory m_resFactory;
    Common::CSimplePool m_simplePool;
    Common::CCharacterFactoryBuilder m_charFactoryBuilder;
    Common::CAiFuncMap m_aiFuncMap;
    Common::CGameState m_gameState;
    Common::CInGameTweakManager m_tweakManager;
    std::unique_ptr<Common::CBooRenderer> m_renderer;

    void AddPaksAndFactories()
    {
    }
    void LoadStringTable()
    {
    }
    static Common::CBooRenderer*
    AllocateRenderer(Common::IObjectStore& store, Common::COsContext& osctx,
                     Common::CMemorySys& memSys, Common::CResFactory& resFactory)
    {
        Common::g_Renderer = new Common::CBooRenderer(store, osctx, memSys, resFactory);
        return Common::g_Renderer;
    }

public:
    CGameGlobalObjects()
    : m_simplePool(m_resFactory)
    {
        Common::g_MemoryCardSys = &m_memoryCardSys;
        Common::g_ResFactory = &m_resFactory;
        Common::g_SimplePool = &m_simplePool;
        Common::g_CharFactoryBuilder = &m_charFactoryBuilder;
        Common::g_AiFuncMap = &m_aiFuncMap;
        Common::g_GameState = &m_gameState;
        Common::g_TweakManager = &m_tweakManager;
    }

    void PostInitialize(Common::COsContext& osctx, Common::CMemorySys& memSys)
    {
        AddPaksAndFactories();
        LoadStringTable();
        m_renderer.reset(AllocateRenderer(m_simplePool, osctx, memSys, m_resFactory));
    }
};

class CGameArchitectureSupport
{
    Common::CAudioSys m_audioSys;
    Common::CInputGenerator m_inputGenerator;
    Common::CGuiSys m_guiSys;
    Common::CIOWinManager m_ioWinManager;
    Common::CSplashScreen m_splashScreen;
    Common::CMainFlow m_mainFlow;
    Common::CConsoleOutputWindow m_consoleWindow;
    Common::CAudioStateWin m_audioStateWin;
public:
    CGameArchitectureSupport(Common::COsContext& osctx)
        : m_audioSys(0,0,0,0,0)
    {
    }
};

class CMain : public Common::COsContext
{
    Common::CMemorySys m_memSys;
    Common::CTweaks m_tweaks;
    bool m_run = true;
public:
    CMain()
    : m_memSys(*this, Common::CMemorySys::GetGameAllocator())
    {
        OpenWindow("", 0, 0, 640, 480);
    }
    void InitializeSubsystems()
    {
        Common::CElementGen::Initialize();
        Common::CAnimData::InitializeCache();
        Common::CDecalManager::Initialize();
    }
    void AddWorldPaks()
    {
    }
    void FillInAssetIDs()
    {
    }
    int RsMain(int argc, const char* argv[])
    {
        Common::TOneStatic<CGameGlobalObjects> globalObjs;
        InitializeSubsystems();
        globalObjs->PostInitialize(*this, m_memSys);
        m_tweaks.RegisterTweaks();
        AddWorldPaks();
        Common::g_TweakManager->ReadFromMemoryCard("AudioTweaks");
        FillInAssetIDs();
        Common::TOneStatic<CGameArchitectureSupport> archSupport(*this);
        while (m_run)
        {

        }
        return 0;
    }
};

}
}

int main(int argc, const char* argv[])
{
    Retro::MP1::CMain main;
    return main.RsMain(argc, argv);
}
