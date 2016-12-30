#include "MP1.hpp"
#include "Graphics/Shaders/CModelShaders.hpp"
#include "Graphics/Shaders/CThermalColdFilter.hpp"
#include "Graphics/Shaders/CThermalHotFilter.hpp"
#include "Graphics/Shaders/CSpaceWarpFilter.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Graphics/Shaders/CXRayBlurFilter.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{
URDE_DECL_SPECIALIZE_SHADER(CThermalColdFilter)
URDE_DECL_SPECIALIZE_SHADER(CThermalHotFilter)
URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)
URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)
URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilterAlpha)

namespace MP1
{

CGameArchitectureSupport::CGameArchitectureSupport(CMain& parent, boo::IAudioVoiceEngine* voiceEngine,
                                                   amuse::IBackendVoiceAllocator& backend)
: m_parent(parent),
  x0_audioSys(voiceEngine, backend, 0,0,0,0,0),
  x30_inputGenerator(g_tweakPlayer->GetLeftLogicalThreshold(),
                   g_tweakPlayer->GetRightLogicalThreshold()),
  x44_guiSys(*g_ResFactory, *g_SimplePool, CGuiSys::EUsageMode::Zero)
{
    g_GuiSys = &x44_guiSys;
    x30_inputGenerator.startScanning();
    g_Main->ResetGameState();

    std::shared_ptr<CIOWin> splash = std::make_shared<CSplashScreen>(CSplashScreen::ESplashScreen::Nintendo);
    x58_ioWinManager.AddIOWin(splash, 1000, 10000);

    std::shared_ptr<CIOWin> mf = std::make_shared<CMainFlow>();
    x58_ioWinManager.AddIOWin(mf, 0, 0);

    std::shared_ptr<CIOWin> console = std::make_shared<CConsoleOutputWindow>(8, 5.f, 0.75f);
    x58_ioWinManager.AddIOWin(console, 100, 0);

    std::shared_ptr<CIOWin> audState = std::make_shared<CAudioStateWin>();
    x58_ioWinManager.AddIOWin(audState, 100, -1);

    std::shared_ptr<CIOWin> errWin = std::make_shared<CErrorOutputWindow>(false);
    x58_ioWinManager.AddIOWin(errWin, 10000, 100000);
}

bool CGameArchitectureSupport::Update()
{
    if (!g_MemoryCardSys)
        m_parent.x128_globalObjects.MemoryCardInitializePump();

    bool finished = false;

    g_GameState->GetWorldTransitionManager()->TouchModels();
    x4_archQueue.Push(MakeMsg::CreateFrameBegin(EArchMsgTarget::Game, x78_));
    x4_archQueue.Push(MakeMsg::CreateTimerTick(EArchMsgTarget::Game, 1.f / 60.f));

    x58_ioWinManager.PumpMessages(x4_archQueue);

    return finished;
}

void CGameArchitectureSupport::Draw()
{
    x58_ioWinManager.Draw();
}

CMain::CMain(IFactory& resFactory, CSimplePool& resStore,
             boo::IGraphicsDataFactory* gfxFactory,
             boo::IGraphicsCommandQueue* cmdQ,
             boo::ITextureR* spareTex)
: m_booSetter(gfxFactory, cmdQ, spareTex),
  x128_globalObjects(resFactory, resStore)
{
    xe4_gameplayResult = EGameplayResult::Playing;
    g_Main = this;
}

CMain::BooSetter::BooSetter(boo::IGraphicsDataFactory* factory,
                            boo::IGraphicsCommandQueue* cmdQ,
                            boo::ITextureR* spareTex)
{
    CGraphics::InitializeBoo(factory, cmdQ, spareTex);
    TShader<CThermalColdFilter>::Initialize();
    TShader<CThermalHotFilter>::Initialize();
    TShader<CSpaceWarpFilter>::Initialize();
    TShader<CCameraBlurFilter>::Initialize();
    TShader<CXRayBlurFilter>::Initialize();
    TMultiBlendShader<CColoredQuadFilter>::Initialize();
    TMultiBlendShader<CTexturedQuadFilter>::Initialize();
    TMultiBlendShader<CTexturedQuadFilterAlpha>::Initialize();
}

void CMain::RegisterResourceTweaks()
{
}
void CMain::ResetGameState()
{
    CPersistentOptions sysOpts = g_GameState->SystemOptions();
    CGameOptions gameOpts = g_GameState->GameOptions();
    x128_globalObjects.ResetGameState();
    g_GameState->ImportPersistentOptions(sysOpts);
    g_GameState->SetGameOptions(gameOpts);
    g_GameState->GetPlayerState()->SetIsFusionEnabled(g_GameState->SystemOptions().GetPlayerHasFusion());
}

void CMain::InitializeSubsystems(const hecl::Runtime::FileStoreManager& storeMgr)
{
    CModelShaders::Initialize(storeMgr, CGraphics::g_BooFactory);
    CMoviePlayer::Initialize();
    CLineRenderer::Initialize();
    CElementGen::Initialize();
    CAnimData::InitializeCache();
    CDecalManager::Initialize();
}

void CMain::FillInAssetIDs()
{
}
void CMain::LoadAudio()
{
}

void CMain::StreamNewGameState(CBitStreamReader& r, u32 idx)
{
    bool fusionBackup = g_GameState->SystemOptions().GetPlayerHasFusion();
    x128_globalObjects.x134_gameState = std::make_unique<CGameState>(r, idx);
    g_GameState = x128_globalObjects.x134_gameState.get();
    g_GameState->SystemOptions().SetPlayerHasFusion(fusionBackup);
    g_GameState->GetPlayerState()->SetIsFusionEnabled(fusionBackup);
    g_GameState->HintOptions().SetNextHintTime();
}

void CMain::Init(const hecl::Runtime::FileStoreManager& storeMgr,
                 boo::IAudioVoiceEngine* voiceEngine,
                 amuse::IBackendVoiceAllocator& backend)
{
    InitializeSubsystems(storeMgr);
    x128_globalObjects.PostInitialize();
    x70_tweaks.RegisterTweaks();
    x70_tweaks.RegisterResourceTweaks();
    m_archSupport.reset(new CGameArchitectureSupport(*this, voiceEngine, backend));
    g_archSupport = m_archSupport.get();
    //g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
}

bool CMain::Proc()
{
    xe8_b24_finished = m_archSupport->Update();
    return xe8_b24_finished;
}

void CMain::Draw()
{
    m_archSupport->Draw();
}

void CMain::Shutdown()
{
    TShader<CThermalColdFilter>::Shutdown();
    TShader<CThermalHotFilter>::Shutdown();
    TShader<CSpaceWarpFilter>::Shutdown();
    TShader<CCameraBlurFilter>::Shutdown();
    TShader<CXRayBlurFilter>::Shutdown();
    TMultiBlendShader<CColoredQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilterAlpha>::Shutdown();
}

#if MP1_USE_BOO

int CMain::appMain(boo::IApplication* app)
{
    zeus::detectCPU();
    mainWindow = app->newWindow(_S("Metroid Prime 1 Reimplementation vZygote"), 1);
    mainWindow->showWindow();
    TOneStatic<CGameGlobalObjects> globalObjs;
    InitializeSubsystems();
    globalObjs->PostInitialize();
    x70_tweaks.RegisterTweaks();
    AddWorldPaks();
    g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
    TOneStatic<CGameArchitectureSupport> archSupport;
    mainWindow->setCallback(archSupport.GetAllocSpace());

    boo::IGraphicsCommandQueue* gfxQ = mainWindow->getCommandQueue();
    boo::SWindowRect windowRect = mainWindow->getWindowFrame();
    boo::ITextureR* renderTex;
    boo::GraphicsDataToken data = mainWindow->getMainContextDataFactory()->commitTransaction(
    [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        renderTex = ctx.newRenderTexture(windowRect.size[0], windowRect.size[1], true, true);
        return true;
    });
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

#endif

}
}
