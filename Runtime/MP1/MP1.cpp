#include "MP1.hpp"
#include "Graphics/Shaders/CModelShaders.hpp"
#include "Graphics/Shaders/CThermalColdFilter.hpp"
#include "Graphics/Shaders/CThermalHotFilter.hpp"
#include "Graphics/Shaders/CSpaceWarpFilter.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Graphics/Shaders/CXRayBlurFilter.hpp"
#include "Graphics/Shaders/CTextSupportShader.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "Audio/CStreamAudioManager.hpp"
#include "CGBASupport.hpp"
#include "CBasics.hpp"
#include "Audio/CAudioGroupSet.hpp"

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
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTextSupportShader)

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
    CMain* m = static_cast<CMain*>(g_Main);

    g_GuiSys = &x44_guiSys;
    x30_inputGenerator.startScanning();

    CAudioSys::SysSetVolume(0x7f);
    CAudioSys::SetDefaultVolumeScale(0x75);
    CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
    CStreamAudioManager::Initialize();
    CStreamAudioManager::SetMusicVolume(0x7f);
    m->ResetGameState();

    //std::shared_ptr<CIOWin> splash = std::make_shared<CSplashScreen>(CSplashScreen::ESplashScreen::Nintendo);
    //x58_ioWinManager.AddIOWin(splash, 1000, 10000);

    std::shared_ptr<CIOWin> mf = std::make_shared<CMainFlow>();
    x58_ioWinManager.AddIOWin(mf, 0, 0);

    std::shared_ptr<CIOWin> console = std::make_shared<CConsoleOutputWindow>(8, 5.f, 0.75f);
    x58_ioWinManager.AddIOWin(console, 100, 0);

    std::shared_ptr<CIOWin> audState = std::make_shared<CAudioStateWin>();
    x58_ioWinManager.AddIOWin(audState, 100, -1);

    std::shared_ptr<CIOWin> errWin = std::make_shared<CErrorOutputWindow>(false);
    x58_ioWinManager.AddIOWin(errWin, 10000, 100000);
}

void CGameArchitectureSupport::UpdateTicks()
{
    x4_archQueue.Push(MakeMsg::CreateTimerTick(EArchMsgTarget::Game, 1.f / 60.f));
}

void CGameArchitectureSupport::Update()
{
    g_GameState->GetWorldTransitionManager()->TouchModels();
    x30_inputGenerator.Update(1 / 60.f, x4_archQueue);
    x4_archQueue.Push(MakeMsg::CreateFrameBegin(EArchMsgTarget::Game, x78_));
    x58_ioWinManager.PumpMessages(x4_archQueue);
}

struct AudioGroupInfo
{
    const char* name;
    u32 id;
};

static const AudioGroupInfo StaticAudioGroups[] =
{
    {"Misc_AGSC", 39},
    {"MiscSamus_AGSC", 41},
    {"UI_AGSC", 40},
    {"Weapons_AGSC", 43},
    {"ZZZ_AGSC", 65}
};

bool CGameArchitectureSupport::LoadAudio()
{
    if (x88_audioLoadStatus == EAudioLoadStatus::Loaded)
        return true;

    for (int i=0 ; i<5 ; ++i)
    {
        TToken<CAudioGroupSet>& tok = x8c_pendingAudioGroups[i];
        if (tok.IsLocked())
        {
            if (tok.IsLoaded())
            {
                CAudioGroupSet* set = tok.GetObj();
                if (!CAudioSys::SysIsGroupSetLoaded(set->GetName()))
                {
                    CAudioSys::SysLoadGroupSet(tok, set->GetName(), tok.GetObjectTag()->id);
                    CAudioSys::SysAddGroupIntoAmuse(set->GetName());
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            /* Lock next pending group */
            tok.Lock();
            return false;
        }
    }

    CSfxManager::LoadTranslationTable(g_SimplePool, g_ResFactory->GetResourceIdByName("sound_lookup"));
    x8c_pendingAudioGroups = std::vector<TToken<CAudioGroupSet>>();
    x88_audioLoadStatus = EAudioLoadStatus::Loaded;

    return true;
}

void CGameArchitectureSupport::PreloadAudio()
{
    if (x88_audioLoadStatus != EAudioLoadStatus::Uninitialized)
        return;
    x8c_pendingAudioGroups.clear();
    x8c_pendingAudioGroups.reserve(5);

    for (int i=0 ; i<5 ; ++i)
    {
        const AudioGroupInfo& info = StaticAudioGroups[i];
        CToken grp = g_SimplePool->GetObj(info.name);
        if (i == 0) /* Lock first group in sequence */
            grp.Lock();
        x8c_pendingAudioGroups.push_back(std::move(grp));
    }

    x88_audioLoadStatus = EAudioLoadStatus::Loading;
}

void CGameArchitectureSupport::UnloadAudio()
{

    for (int i=0 ; i<5 ; ++i)
    {
        const AudioGroupInfo& info = StaticAudioGroups[i];
        const SObjectTag* tag = g_ResFactory->GetResourceIdByName(info.name);
        const std::string& name = CAudioSys::SysGetGroupSetName(tag->id);
        CAudioSys::SysRemoveGroupFromAmuse(name);
        CAudioSys::SysUnloadAudioGroupSet(name);
    }

    x8c_pendingAudioGroups = std::vector<TToken<CAudioGroupSet>>();
    x88_audioLoadStatus = EAudioLoadStatus::Uninitialized;
}

void CGameArchitectureSupport::Draw()
{
    x58_ioWinManager.Draw();
}

CGameArchitectureSupport::~CGameArchitectureSupport()
{
    x58_ioWinManager.RemoveAllIOWins();
    UnloadAudio();
    CSfxManager::Shutdown();
    CStreamAudioManager::Shutdown();
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
    TMultiBlendShader<CTextSupportShader>::Initialize();
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
    g_GameState->GetPlayerState()->SetIsFusionEnabled(g_GameState->SystemOptions().GetPlayerFusionSuitActive());
}

void CMain::InitializeSubsystems(const hecl::Runtime::FileStoreManager& storeMgr)
{
    CBasics::Initialize();
    CModelShaders::Initialize(storeMgr, CGraphics::g_BooFactory);
    CMoviePlayer::Initialize();
    CLineRenderer::Initialize();
    CElementGen::Initialize();
    CAnimData::InitializeCache();
    CDecalManager::Initialize();
    CGBASupport::Initialize();
}

void CMain::FillInAssetIDs()
{
}

bool CMain::LoadAudio()
{
    if (x164_archSupport)
        return x164_archSupport->LoadAudio();
    return true;
}

void CMain::StreamNewGameState(CBitStreamReader& r, u32 idx)
{
    bool fusionBackup = g_GameState->SystemOptions().GetPlayerFusionSuitActive();
    x128_globalObjects.x134_gameState = std::make_unique<CGameState>(r, idx);
    g_GameState = x128_globalObjects.x134_gameState.get();
    g_GameState->SystemOptions().SetPlayerFusionSuitActive(fusionBackup);
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
    FillInAssetIDs();
    x164_archSupport.reset(new CGameArchitectureSupport(*this, voiceEngine, backend));
    g_archSupport = x164_archSupport.get();
    x164_archSupport->PreloadAudio();
    //g_TweakManager->ReadFromMemoryCard("AudioTweaks");

    //CStreamAudioManager::Start(false, "Audio/rui_samusL.dsp|Audio/rui_samusR.dsp", 0x7f, true, 1.f, 1.f);
}

bool CMain::Proc()
{
    CGBASupport::GlobalPoll();
    x164_archSupport->UpdateTicks();
    x164_archSupport->Update();
    CSfxManager::Update(1.f / 60.f);
    CStreamAudioManager::Update(1.f / 60.f);
    if (x164_archSupport->GetIOWinManager().IsEmpty() || CheckReset())
    {
        CStreamAudioManager::StopAll();
        /*
        x164_archSupport.reset();
        g_archSupport = x164_archSupport.get();
        x164_archSupport->PreloadAudio();
        */
        x160_24_finished = true;
    }
    return x160_24_finished;
}

void CMain::Draw()
{
    x164_archSupport->Draw();
}

void CMain::ShutdownSubsystems()
{
    CMoviePlayer::Shutdown();
    CLineRenderer::Shutdown();
    CDecalManager::Shutdown();
    CElementGen::Shutdown();
    CAnimData::FreeCache();
    CMemoryCardSys::Shutdown();
}

void CMain::Shutdown()
{
    x164_archSupport.reset();
    ShutdownSubsystems();
    TShader<CThermalColdFilter>::Shutdown();
    TShader<CThermalHotFilter>::Shutdown();
    TShader<CSpaceWarpFilter>::Shutdown();
    TShader<CCameraBlurFilter>::Shutdown();
    TShader<CXRayBlurFilter>::Shutdown();
    TMultiBlendShader<CColoredQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilterAlpha>::Shutdown();
    TMultiBlendShader<CTextSupportShader>::Shutdown();
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
