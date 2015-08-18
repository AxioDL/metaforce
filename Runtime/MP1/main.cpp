#include <memory>
#include <boo/boo.hpp>
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
CMemoryCardSys* g_MemoryCardSys = nullptr;
CResFactory* g_ResFactory = nullptr;
CSimplePool* g_SimplePool = nullptr;
CCharacterFactoryBuilder* g_CharFactoryBuilder = nullptr;
CAiFuncMap* g_AiFuncMap = nullptr;
CGameState* g_GameState = nullptr;
CInGameTweakManager* g_TweakManager = nullptr;
CBooRenderer* g_Renderer = nullptr;

namespace MP1
{

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
    AllocateRenderer(IObjectStore& store, COsContext& osctx,
                     CMemorySys& memSys, CResFactory& resFactory)
    {
        g_Renderer = new CBooRenderer(store, osctx, memSys, resFactory);
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

    void PostInitialize(COsContext& osctx, CMemorySys& memSys)
    {
        AddPaksAndFactories();
        LoadStringTable();
        m_renderer.reset(AllocateRenderer(m_simplePool, osctx, memSys, m_resFactory));
    }
};

class CGameArchitectureSupport
{
    CAudioSys m_audioSys;
    CInputGenerator m_inputGenerator;
    CGuiSys m_guiSys;
    CIOWinManager m_ioWinManager;
    CSplashScreen m_splashScreen;
    CMainFlow m_mainFlow;
    CConsoleOutputWindow m_consoleWindow;
    CAudioStateWin m_audioStateWin;
public:
    CGameArchitectureSupport(COsContext& osctx)
        : m_audioSys(0,0,0,0,0)
    {
    }
};

class CMain : public COsContext
{
    CMemorySys m_memSys;
    CTweaks m_tweaks;
    bool m_run = true;
public:
    CMain()
    : m_memSys(*this, CMemorySys::GetGameAllocator())
    {
        OpenWindow("", 0, 0, 640, 480);
    }
    void InitializeSubsystems()
    {
        CElementGen::Initialize();
        CAnimData::InitializeCache();
        CDecalManager::Initialize();
    }
    void AddWorldPaks()
    {
    }
    void FillInAssetIDs()
    {
    }
    int RsMain(int argc, const char* argv[])
    {
        TOneStatic<CGameGlobalObjects> globalObjs;
        InitializeSubsystems();
        globalObjs->PostInitialize(*this, m_memSys);
        m_tweaks.RegisterTweaks();
        AddWorldPaks();
        g_TweakManager->ReadFromMemoryCard("AudioTweaks");
        FillInAssetIDs();
        TOneStatic<CGameArchitectureSupport> archSupport(*this);
        while (m_run)
        {

        }
        return 0;
    }
};

}
}

struct WindowCallback : boo::IWindowCallback
{

    void mouseDown(const SWindowCoord& coord, EMouseButton button, EModifierKey mods)
    {
        fprintf(stderr, "Mouse Down %d (%f,%f)\n", button, coord.norm[0], coord.norm[1]);
    }
    void mouseUp(const SWindowCoord& coord, EMouseButton button, EModifierKey mods)
    {
        fprintf(stderr, "Mouse Up %d (%f,%f)\n", button, coord.norm[0], coord.norm[1]);
    }
    void mouseMove(const SWindowCoord& coord)
    {
        //fprintf(stderr, "Mouse Move (%f,%f)\n", coord.norm[0], coord.norm[1]);
    }
    void scroll(const SWindowCoord& coord, const SScrollDelta& scroll)
    {
        fprintf(stderr, "Mouse Scroll (%f,%f) (%f,%f)\n", coord.norm[0], coord.norm[1], scroll.delta[0], scroll.delta[1]);
    }

    void touchDown(const STouchCoord& coord, uintptr_t tid)
    {
        //fprintf(stderr, "Touch Down %16lX (%f,%f)\n", tid, coord.coord[0], coord.coord[1]);
    }
    void touchUp(const STouchCoord& coord, uintptr_t tid)
    {
        //fprintf(stderr, "Touch Up %16lX (%f,%f)\n", tid, coord.coord[0], coord.coord[1]);
    }
    void touchMove(const STouchCoord& coord, uintptr_t tid)
    {
        //fprintf(stderr, "Touch Move %16lX (%f,%f)\n", tid, coord.coord[0], coord.coord[1]);
    }

    void charKeyDown(unsigned long charCode, EModifierKey mods, bool isRepeat)
    {

    }
    void charKeyUp(unsigned long charCode, EModifierKey mods)
    {

    }
    void specialKeyDown(ESpecialKey key, EModifierKey mods, bool isRepeat)
    {

    }
    void specialKeyUp(ESpecialKey key, EModifierKey mods)
    {

    }
    void modKeyDown(EModifierKey mod, bool isRepeat)
    {

    }
    void modKeyUp(EModifierKey mod)
    {

    }

};

struct DolphinSmashAdapterCallback : boo::IDolphinSmashAdapterCallback
{
    void controllerConnected(unsigned idx, boo::EDolphinControllerType)
    {
        printf("CONTROLLER %u CONNECTED\n", idx);
    }
    void controllerDisconnected(unsigned idx, boo::EDolphinControllerType)
    {
        printf("CONTROLLER %u DISCONNECTED\n", idx);
    }
    void controllerUpdate(unsigned idx, boo::EDolphinControllerType,
                          const boo::DolphinControllerState& state)
    {
        printf("CONTROLLER %u UPDATE %d %d\n", idx, state.m_leftStick[0], state.m_leftStick[1]);
    }
};

class ApplicationDeviceFinder : public boo::DeviceFinder
{
    std::unique_ptr<boo::DolphinSmashAdapter> smashAdapter = NULL;
    DolphinSmashAdapterCallback m_cb;
public:
    ApplicationDeviceFinder()
    : boo::DeviceFinder({typeid(boo::DolphinSmashAdapter)})
    {}
    void deviceConnected(boo::DeviceToken& tok)
    {
        if (!smashAdapter)
        {
            smashAdapter.reset(dynamic_cast<boo::DolphinSmashAdapter*>(tok.openAndGetDevice()));
            smashAdapter->setCallback(&m_cb);
            smashAdapter->startRumble(0);
        }
    }
    void deviceDisconnected(boo::DeviceToken&, boo::DeviceBase* device)
    {
        if (smashAdapter.get() == device)
            smashAdapter.reset(nullptr);
    }
};

struct ApplicationCallback : boo::IApplicationCallback
{
    boo::IWindow* mainWindow = NULL;
    ApplicationDeviceFinder devFinder;
    WindowCallback windowCallback;
    void appLaunched(boo::IApplication* app)
    {
        mainWindow = app->newWindow("YAY!");
        mainWindow->setCallback(&windowCallback);
        mainWindow->showWindow();
        devFinder.startScanning();
    }
    void appQuitting(boo::IApplication*)
    {
        delete mainWindow;
    }
    void appFilesOpen(boo::IApplication*, const std::vector<std::string>& paths)
    {
        fprintf(stderr, "OPENING: ");
        for (const std::string& path : paths)
            fprintf(stderr, "%s ", path.c_str());
        fprintf(stderr, "\n");
    }
};

int main(int argc, const char* argv[])
{
    Retro::TOneStatic<ApplicationCallback> appCB;
    std::unique_ptr<boo::IApplication> app =
        boo::ApplicationBootstrap(boo::IApplication::PLAT_AUTO, *appCB,
                                  "mp1", "MP1", argc, argv);
    Retro::TOneStatic<Retro::MP1::CMain> main;
    return main->RsMain(argc, argv);
}
