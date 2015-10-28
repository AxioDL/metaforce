#if _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#endif

#include <clocale>
#include <memory>
#include <boo/boo.hpp>
#include "CBasics.hpp"
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
#include "CArchitectureQueue.hpp"
#include "CMain.hpp"

#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"

namespace Retro
{
CMemoryCardSys* g_MemoryCardSys = nullptr;
CResFactory* g_ResFactory = nullptr;
CSimplePool* g_SimplePool = nullptr;
CCharacterFactoryBuilder* g_CharFactoryBuilder = nullptr;
CAiFuncMap* g_AiFuncMap = nullptr;
CGameState* g_GameState = nullptr;
CInGameTweakManagerBase* g_TweakManager = nullptr;
CBooRenderer* g_Renderer = nullptr;

namespace MP1
{
class CMain* g_main = nullptr;

class CGameGlobalObjects
{
    CMemoryCardSys m_memoryCardSys;
    CResFactory m_resFactory;
    CSimplePool m_simplePool;
    CCharacterFactoryBuilder m_charFactoryBuilder;
    CAiFuncMap m_aiFuncMap;
    CGameState m_gameState;
    CInGameTweakManager m_tweakManager;
    std::unique_ptr<CBooRenderer> m_renderer;

    void AddPaksAndFactories()
    {
    }
    void LoadStringTable()
    {
    }
    static CBooRenderer*
    AllocateRenderer(IObjectStore& store,
                     CMemorySys& memSys, CResFactory& resFactory)
    {
        g_Renderer = new CBooRenderer(store, memSys, resFactory);
        return g_Renderer;
    }

public:
    CGameGlobalObjects()
    : m_simplePool(m_resFactory)
    {
        g_MemoryCardSys = &m_memoryCardSys;
        g_ResFactory = &m_resFactory;
        g_SimplePool = &m_simplePool;
        g_CharFactoryBuilder = &m_charFactoryBuilder;
        g_AiFuncMap = &m_aiFuncMap;
        g_GameState = &m_gameState;
        g_TweakManager = &m_tweakManager;
    }

    void PostInitialize(CMemorySys& memSys)
    {
        AddPaksAndFactories();
        LoadStringTable();
        m_renderer.reset(AllocateRenderer(m_simplePool, memSys, m_resFactory));
    }
};

class CGameArchitectureSupport
{
    CArchitectureQueue m_archQueue;
    CAudioSys m_audioSys;
    CInputGenerator m_inputGenerator;
    CGuiSys m_guiSys;
    CIOWinManager m_ioWinManager;
    CSplashScreen m_splashScreen;
    CMainFlow m_mainFlow;
    CConsoleOutputWindow m_consoleWindow;
    CAudioStateWin m_audioStateWin;
public:
    CGameArchitectureSupport()
        : m_audioSys(0,0,0,0,0),
          m_inputGenerator(0.0f /*g_tweakPlayer->GetLeftLogicalThreshold()*/,
                           0.0f /*g_tweakPlayer->GetRightLogicalThreshold()*/)
    {
    }
    bool Update()
    {
        bool finished = false;
        return finished;
    }
};

CMain::CMain()
: x6c_memSys(CMemorySys::GetGameAllocator())
{
    g_main = this;
    xe4_gameplayResult = GameplayResultPlaying;
}
void CMain::RegisterResourceTweaks()
{
}
void CMain::ResetGameState()
{
}
void CMain::InitializeSubsystems()
{
    CElementGen::Initialize();
    CAnimData::InitializeCache();
    CDecalManager::Initialize();
}
void CMain::AddWorldPaks()
{
}
void CMain::FillInAssetIDs()
{
}
void CMain::LoadAudio()
{
}
int CMain::RsMain(int argc, const boo::SystemChar* argv[])
{
    TOneStatic<CGameGlobalObjects> globalObjs;
    InitializeSubsystems();
    globalObjs->PostInitialize(x6c_memSys);
    x70_tweaks.RegisterTweaks();
    AddWorldPaks();
    g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
    TOneStatic<CGameArchitectureSupport> archSupport;
    while (!xe8_b24_finished)
    {
        xe8_b24_finished = archSupport->Update();
    }
    return 0;
}

}
}

#ifdef _WIN32
int wmain(int argc, const wchar_t* argv[])
#else
int main(int argc, const char* argv[])
#endif
{
#if _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#else
    std::setlocale(LC_ALL, "en-US.UTF-8");
#endif
    
    LogVisor::RegisterConsoleLogger();
    Retro::TOneStatic<Retro::MP1::CMain> main;
    std::unique_ptr<boo::IApplication> app =
        boo::ApplicationBootstrap(boo::IApplication::PLAT_AUTO, *main,
                                  _S("mp1"), _S("MP1"), argc, argv);
    return main->RsMain(argc, argv);
}
