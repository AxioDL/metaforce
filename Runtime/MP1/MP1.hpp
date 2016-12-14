#ifndef __URDE_MP1_HPP__
#define __URDE_MP1_HPP__

#define MP1_USE_BOO 0

#include "IMain.hpp"
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
#include "GuiSys/CErrorOutputWindow.hpp"
#include "GuiSys/CTextParser.hpp"
#include "Audio/CAudioStateWin.hpp"
#include "GameGlobalObjects.hpp"
#include "CArchitectureQueue.hpp"
#include "CTimeProvider.hpp"
#include "GuiSys/CTextExecuteBuffer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"

namespace urde
{
class IFactory;
class IObjectStore;

namespace MP1
{
class CMain;

class CGameGlobalObjects
{
    std::unique_ptr<CMemoryCardSys> x0_memoryCardSys;
    IFactory& x4_resFactory;
    CSimplePool& xcc_simplePool;
    CCharacterFactoryBuilder xec_charFactoryBuilder;
    CAiFuncMap x110_aiFuncMap;
    CGameState x134_gameState;
    TLockedToken<CStringTable> x13c_mainStringTable;
    CInGameTweakManager x150_tweakManager;
    std::unique_ptr<CBooRenderer> m_renderer;

    void LoadStringTable()
    {
        x13c_mainStringTable = g_SimplePool->GetObj("STRG_Main");
        g_MainStringTable = x13c_mainStringTable.GetObj();
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
    : x4_resFactory(resFactory), xcc_simplePool(objStore)
    {
        g_ResFactory = &x4_resFactory;
        g_SimplePool = &xcc_simplePool;
        g_CharFactoryBuilder = &xec_charFactoryBuilder;
        g_AiFuncMap = &x110_aiFuncMap;
        g_GameState = &x134_gameState;
        g_TweakManager = &x150_tweakManager;
    }

    void PostInitialize()
    {
        LoadStringTable();
        m_renderer.reset(AllocateRenderer(xcc_simplePool, x4_resFactory));
    }

    void MemoryCardInitializePump()
    {
        if (!g_MemoryCardSys)
        {
            if (!x0_memoryCardSys)
                x0_memoryCardSys.reset(new CMemoryCardSys());
            if (x0_memoryCardSys->InitializePump())
                g_MemoryCardSys = x0_memoryCardSys.get();
        }
    }
};

#if MP1_USE_BOO
class CGameArchitectureSupport : public boo::IWindowCallback
#else
class CGameArchitectureSupport
#endif
{
    CMain& m_parent;
    CArchitectureQueue m_archQueue;
    CAudioSys m_audioSys;
    CInputGenerator m_inputGenerator;
    CGuiSys m_guiSys;
    CIOWinManager m_ioWinManager;
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

    void destroyed() { m_archQueue.Push(MakeMsg::CreateApplicationExit(EArchMsgTarget::ArchitectureSupport)); }

    void resized(const boo::SWindowRect &rect)
    {
        m_windowRect = rect;
        m_rectIsDirty = true;
    }

public:
    CGameArchitectureSupport(CMain& parent, boo::IAudioVoiceEngine* voiceEngine,
                             amuse::IBackendVoiceAllocator& backend);
    void PreloadAudio();
    bool Update();
    void Draw();

    bool isRectDirty() { return m_rectIsDirty; }
    const boo::SWindowRect& getWindowRect()
    {
        m_rectIsDirty = false;
        return m_windowRect;
    }
};

#if MP1_USE_BOO
class CMain : public boo::IApplicationCallback, public IMain
#else
class CMain : public IMain
#endif
{
    friend class CGameArchitectureSupport;
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
    std::unique_ptr<CGameArchitectureSupport> m_archSupport;

    EFlowState x12c_flowState = EFlowState::Five;

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

    void InitializeSubsystems(const hecl::Runtime::FileStoreManager& storeMgr);

public:
    CMain(IFactory& resFactory, CSimplePool& resStore,
          boo::IGraphicsDataFactory* gfxFactory,
          boo::IGraphicsCommandQueue* cmdQ,
          boo::ITextureR* spareTex);
    void RegisterResourceTweaks();
    void ResetGameState();
    void StreamNewGameState(CInputStream&) {}
    void CheckTweakManagerDebugOptions() {}

    //int RsMain(int argc, const boo::SystemChar* argv[]);
    void Init(const hecl::Runtime::FileStoreManager& storeMgr,
              boo::IAudioVoiceEngine* voiceEngine,
              amuse::IBackendVoiceAllocator& backend);
    bool Proc();
    void Draw();
    void Shutdown();

    bool CheckReset() { return false; }
    bool CheckTerminate() {return false;}
    void DrawDebugMetrics(double, CStopWatch&) {}
    void DoPredrawMetrics() {}
    void FillInAssetIDs();
    void LoadAudio();
    void ShutdownSubsystems() {}
    EGameplayResult GetGameplayResult() const {return xe4_gameplayResult;}
    void SetGameplayResult(EGameplayResult wl) {xe4_gameplayResult = wl;}

    EFlowState GetFlowState() const { return x12c_flowState; }
};

}
}

#endif // __URDE_MP1_HPP__
