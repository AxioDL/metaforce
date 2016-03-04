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
#include "MP1.hpp"
#include "CTimeProvider.hpp"

#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"

namespace pshag
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
    CMemoryCardSys x0_memoryCardSys;
    CResFactory x20_resFactory;
    CSimplePool x114_simplePool;
    CCharacterFactoryBuilder x14c_charFactoryBuilder;
    CAiFuncMap x188_aiFuncMap;
    CGameState x1a8_gameState;
    CInGameTweakManager x1c0_tweakManager;
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
    : x114_simplePool(x20_resFactory)
    {
        g_MemoryCardSys = &x0_memoryCardSys;
        g_ResFactory = &x20_resFactory;
        g_SimplePool = &x114_simplePool;
        g_CharFactoryBuilder = &x14c_charFactoryBuilder;
        g_AiFuncMap = &x188_aiFuncMap;
        g_GameState = &x1a8_gameState;
        g_TweakManager = &x1c0_tweakManager;
    }

    void PostInitialize(CMemorySys& memSys)
    {
        AddPaksAndFactories();
        LoadStringTable();
        m_renderer.reset(AllocateRenderer(x114_simplePool, memSys, x20_resFactory));
    }
};

class CGameArchitectureSupport : public boo::IWindowCallback
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
    boo::SWindowRect m_windowRect;
    bool m_rectIsDirty;

    void mouseDown(const boo::SWindowCoord &coord, boo::EMouseButton button, boo::EModifierKey mods)
    { m_inputGenerator.mouseDown(coord, button, mods); }
    void mouseUp(const boo::SWindowCoord &coord, boo::EMouseButton button, boo::EModifierKey mods)
    { m_inputGenerator.mouseUp(coord, button, mods); }
    void mouseMove(const boo::SWindowCoord &coord)
    { m_inputGenerator.mouseMove(coord); }
    void scroll(const boo::SWindowCoord &coord, const boo::SScrollDelta &scroll)
    { m_inputGenerator.scroll(coord, scroll); }
    void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
    { m_inputGenerator.charKeyDown(charCode, mods, isRepeat); }
    void charKeyUp(unsigned long charCode, boo::EModifierKey mods)
    { m_inputGenerator.charKeyUp(charCode, mods); }
    void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
    { m_inputGenerator.specialKeyDown(key, mods, isRepeat); }
    void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
    { m_inputGenerator.specialKeyUp(key, mods); }
    void modKeyDown(boo::EModifierKey mod, bool isRepeat)
    { m_inputGenerator.modKeyDown(mod, isRepeat);}
    void modKeyUp(boo::EModifierKey mod)
    { m_inputGenerator.modKeyUp(mod); }

    void destroyed() { m_archQueue.Push(std::move(MakeMsg::CreateApplicationExit(EArchMsgTarget::ArchitectureSupport))); }

    void resized(const boo::SWindowRect &rect)
    {
        m_windowRect = rect;
        m_rectIsDirty = true;
    }

public:
    CGameArchitectureSupport()
        : m_audioSys(0,0,0,0,0),
          m_inputGenerator(0.0f /*g_tweakPlayer->GetLeftLogicalThreshold()*/,
                           0.0f /*g_tweakPlayer->GetRightLogicalThreshold()*/)
    {
        m_inputGenerator.startScanning();
    }
    bool Update()
    {
        bool finished = false;
        m_inputGenerator.Update(1.0 / 60.0, m_archQueue);

        while(m_archQueue)
        {
            CArchitectureMessage msg = m_archQueue.Pop();
            if (msg.GetTarget() == EArchMsgTarget::ArchitectureSupport)
            {
                if (msg.GetType() == EArchMsgType::ApplicationExit)
                    finished = true;
            }

            if (msg.GetTarget() == EArchMsgTarget::Game && msg.GetType() == EArchMsgType::UserInput)
            {
                const CArchMsgParmUserInput* input = msg.GetParm<CArchMsgParmUserInput>();
                if (input->x4_parm.DStart())
                    m_archQueue.Push(std::move(MakeMsg::CreateApplicationExit(EArchMsgTarget::ArchitectureSupport)));
            }
        }
        return finished;
    }

    bool isRectDirty() { return m_rectIsDirty; }
    const boo::SWindowRect& getWindowRect()
    {
        m_rectIsDirty = false;
        return m_windowRect;
    }
};

CMain::CMain()
: x6c_memSys(CMemorySys::GetGameAllocator())
{
    g_main = this;
    xe4_gameplayResult = EGameplayResult::Playing;
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
        if (CDvdFile::FileExists(pakName.c_str()))
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
    mainWindow = app->newWindow(_S("Metroid Prime 1 Reimplementation vZygote"), 1);
    mainWindow->showWindow();
    TOneStatic<CGameGlobalObjects> globalObjs;
    InitializeSubsystems();
    globalObjs->PostInitialize(x6c_memSys);
    x70_tweaks.RegisterTweaks();
    AddWorldPaks();
    g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
    TOneStatic<CGameArchitectureSupport> archSupport;
    mainWindow->setCallback(archSupport.GetAllocSpace());

    boo::IGraphicsCommandQueue* gfxQ = mainWindow->getCommandQueue();
    boo::SWindowRect windowRect = mainWindow->getWindowFrame();
    boo::ITextureR* renderTex = mainWindow->getMainContextDataFactory()->newRenderTexture(windowRect.size[0], windowRect.size[1], true, true);
    float rgba[4] = { 0.2f, 0.2f, 0.2f, 1.0f};
    gfxQ->setClearColor(rgba);

    while (!xe8_b24_finished)
    {
        xe8_b24_finished = archSupport->Update();

        if (archSupport->isRectDirty())
        {
            const boo::SWindowRect& windowRect = archSupport->getWindowRect();
            gfxQ->resizeRenderTexture(renderTex,
                                      windowRect.size[0],
                                      windowRect.size[1]);
        }

        gfxQ->setRenderTarget(renderTex);
        gfxQ->clearTarget();
        gfxQ->resolveDisplay(renderTex);
        gfxQ->execute();
        mainWindow->waitForRetrace();
    }
    return 0;
}

}
}
