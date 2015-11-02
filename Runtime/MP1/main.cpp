#if _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#endif

#include <clocale>
#include <memory>
#include <boo/boo.hpp>
#include <boo/graphicsdev/GL.hpp>
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
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"

namespace Retro
{
CMemoryCardSys* g_MemoryCardSys = nullptr;
CResFactory*  g_ResFactory = nullptr;
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
#if 0
    u32 i = 0;
    while (i <= 255)
    {
        std::string pakName = CBasics::Stringize("%s%i.pak", g_tweakGame->GetWorldPrefix().c_str(), i);
        if (!CDvdFile::FileExists(pakName.c_str()))
        {
            i++;
            continue;
        }

        g_ResFactory->GetLoader().AddPakFile(pakName, false);
        i++;
    }
#endif
}

void CMain::FillInAssetIDs()
{
}
void CMain::LoadAudio()
{
}

int CMain::appMain(boo::IApplication* app)
{
    Zeus::detectCPU();
    mainWindow = app->newWindow("Metroid Prime 1 Reimplementation vZygote");
    mainWindow->setCallback(&windowCallback);
    mainWindow->showWindow();
    //devFinder.startScanning();
    TOneStatic<CGameGlobalObjects> globalObjs;
    InitializeSubsystems();
    globalObjs->PostInitialize(x6c_memSys);
    x70_tweaks.RegisterTweaks();
    AddWorldPaks();
    g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
    TOneStatic<CGameArchitectureSupport> archSupport;

    boo::IGraphicsCommandQueue* gfxQ = mainWindow->getCommandQueue();
    float rgba[4] = { 0.5f, 0.5f, 0.5f, 1.0f};
    gfxQ->setClearColor(rgba);

    while (!xe8_b24_finished)
    {
        mainWindow->waitForRetrace();
        xe8_b24_finished = archSupport->Update();
        gfxQ->clearTarget();

        gfxQ->present();
        gfxQ->execute();
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
    int ret = boo::ApplicationRun(boo::IApplication::PLAT_AUTO, *main,
                                  _S("mp1"), _S("MP1"), argc, argv);
    return ret;
}
