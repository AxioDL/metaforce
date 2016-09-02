#ifndef __URDE_MP1_HPP__
#define __URDE_MP1_HPP__

#define MP1_USE_BOO 0

#include <boo/boo.hpp>
#include <boo/graphicsdev/GL.hpp>
#include <boo/audiodev/IAudioVoiceEngine.hpp>
#include "CMemory.hpp"
#include "CTweaks.hpp"
#include "CPlayMovie.hpp"
#include "IOStreams.hpp"
#include "CBasics.hpp"
#include "CMemoryCardSys.hpp"
#include "CResFactory.hpp"
#include "CSimplePool.hpp"
#include "Character/CAssetFactory.hpp"
#include "World/CAi.hpp"
#include "CGameState.hpp"
#include "CInGameTweakManager.hpp"
#include "Particle/CElementGen.hpp"
#include "Character/CAnimData.hpp"
#include "Particle/CDecalManager.hpp"
#include "Particle/CGenDescription.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "Audio/CAudioSys.hpp"
#include "Input/CInputGenerator.hpp"
#include "GuiSys/CGuiSys.hpp"
#include "CIOWinManager.hpp"
#include "GuiSys/CSplashScreen.hpp"
#include "CMainFlow.hpp"
#include "GuiSys/CConsoleOutputWindow.hpp"
#include "GuiSys/CTextParser.hpp"
#include "Audio/CAudioStateWin.hpp"
#include "GameGlobalObjects.hpp"
#include "CArchitectureQueue.hpp"
#include "MP1.hpp"
#include "CTimeProvider.hpp"
#include "GuiSys/CTextExecuteBuffer.hpp"

#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"

namespace urde
{
class CStopwatch;
class IFactory;
class IObjectStore;

namespace MP1
{

enum class EGameplayResult
{
    None,
    Win,
    Lose,
    Playing
};

class CGameGlobalObjects
{
    CMemoryCardSys x0_memoryCardSys;
    IFactory& x20_resFactory;
    CSimplePool& x114_simplePool;
    CCharacterFactoryBuilder x14c_charFactoryBuilder;
    CAiFuncMap x188_aiFuncMap;
    CGameState x1a8_gameState;
    CInGameTweakManager x1c0_tweakManager;
    std::unique_ptr<CBooRenderer> m_renderer;

    void LoadStringTable()
    {
    }
    static CBooRenderer*
    AllocateRenderer(IObjectStore& store, IFactory& resFactory)
    {
        g_Renderer = new CBooRenderer(store, resFactory);
        return g_Renderer;
    }

public:
    CGameGlobalObjects(IFactory& resFactory,
                       CSimplePool& objStore)
    : x20_resFactory(resFactory), x114_simplePool(objStore)
    {
        g_MemoryCardSys = &x0_memoryCardSys;
        g_ResFactory = &x20_resFactory;
        g_SimplePool = &x114_simplePool;
        g_CharFactoryBuilder = &x14c_charFactoryBuilder;
        g_AiFuncMap = &x188_aiFuncMap;
        g_GameState = &x1a8_gameState;
        g_TweakManager = &x1c0_tweakManager;
    }

    void PostInitialize()
    {
        LoadStringTable();
        m_renderer.reset(AllocateRenderer(x114_simplePool, x20_resFactory));
    }
};

#if MP1_USE_BOO
class CGameArchitectureSupport : public boo::IWindowCallback
#else
class CGameArchitectureSupport
#endif
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
                           0.0f /*g_tweakPlayer->GetRightLogicalThreshold()*/),
          m_guiSys(*g_ResFactory, *g_SimplePool, CGuiSys::EUsageMode::Zero)
    {
        g_GuiSys = &m_guiSys;
        m_inputGenerator.startScanning();
    }
    bool Update()
    {
        bool finished = false;
        m_inputGenerator.Update(1.0 / 60.0, m_archQueue);

        g_GameState->GetWorldTransitionManager()->TouchModels();
        int unk = 0;
        m_archQueue.Push(std::move(MakeMsg::CreateFrameBegin(EArchMsgTarget::Game, unk)));

        m_ioWinManager.PumpMessages(m_archQueue);

        /*
        while (m_archQueue)
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
        */
        return finished;
    }

    bool isRectDirty() { return m_rectIsDirty; }
    const boo::SWindowRect& getWindowRect()
    {
        m_rectIsDirty = false;
        return m_windowRect;
    }
};

#if MP1_USE_BOO
class CMain : public boo::IApplicationCallback
#else
class CMain
#endif
{
#if MP1_USE_BOO
    boo::IWindow* mainWindow;
    int appMain(boo::IApplication* app);
    void appQuitting(boo::IApplication*)
    { xe8_b24_finished = true; }
    void appFilesOpen(boo::IApplication*, const std::vector<std::string>& paths)
    {
        fprintf(stderr, "OPENING: ");
        for (const std::string& path : paths)
            fprintf(stderr, "%s ", path.c_str());
        fprintf(stderr, "\n");
    }
#endif
public:
    enum class FlowState
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five,
        Six,
    };
private:

    struct BooSetter
    {
        BooSetter(boo::IGraphicsDataFactory* factory,
                  boo::IGraphicsCommandQueue* cmdQ,
                  boo::ITextureR* spareTex);
    } m_booSetter;

    //CMemorySys x6c_memSys;
    CTweaks x70_tweaks;
    EGameplayResult xe4_gameplayResult;
    bool xe8_b24_finished = false;

    /* urde addition: these are simply initialized along with everything else */
    CGameGlobalObjects x128_globalObjects;
    CGameArchitectureSupport m_archSupport;

    FlowState x12c_ = FlowState::Five;

    u32 x130_[10] = { 1000000 };

    union
    {
        struct
        {
            bool x160_24_ : 1;
            bool x160_25_ : 1;
            bool x160_26_ : 1;
            bool x160_27_ : 1;
            bool x160_28_ : 1;
            bool x160_29_ : 1;
            bool x160_30_ : 1;
            bool x160_31_ : 1;
            bool x161_24_ : 1;
        };
        u16 _dummy = 0;
    };

    u32 x164_ = 0;

    void InitializeSubsystems(const hecl::Runtime::FileStoreManager& storeMgr,
                              boo::IAudioVoiceEngine* voiceEngine);

public:
    CMain(IFactory& resFactory, CSimplePool& resStore,
          boo::IGraphicsDataFactory* gfxFactory,
          boo::IGraphicsCommandQueue* cmdQ,
          boo::ITextureR* spareTex);
    void RegisterResourceTweaks();
    void ResetGameState();
    void StreamNewGameState(CInputStream&);
    void CheckTweakManagerDebugOptions() {}

    //int RsMain(int argc, const boo::SystemChar* argv[]);
    void Init(const hecl::Runtime::FileStoreManager& storeMgr,
              boo::IAudioVoiceEngine* voiceEngine);
    bool Proc();
    void Shutdown();

    bool CheckReset();
    bool CheckTerminate() {return false;}
    void DrawDebugMetrics(double, CStopwatch&) {}
    void DoPredrawMetrics() {}
    void FillInAssetIDs();
    void LoadAudio();
    void ShutdownSubsystems();
    EGameplayResult GetGameplayResult() const {return xe4_gameplayResult;}
    void SetGameplayResult(EGameplayResult wl) {xe4_gameplayResult = wl;}

    FlowState GetFlowState() const { return x12c_; }
};

}
}

#endif // __URDE_MP1_HPP__
