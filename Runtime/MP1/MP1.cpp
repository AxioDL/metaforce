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
#include "Graphics/Shaders/CEnergyBarShader.hpp"
#include "Graphics/Shaders/CRadarPaintShader.hpp"
#include "Graphics/Shaders/CMapSurfaceShader.hpp"
#include "Graphics/Shaders/CPhazonSuitFilter.hpp"
#include "Graphics/Shaders/CScanLinesFilter.hpp"
#include "Graphics/Shaders/CRandomStaticFilter.hpp"
#include "Graphics/Shaders/CFluidPlaneShader.hpp"
#include "Graphics/Shaders/CAABoxShader.hpp"
#include "Graphics/Shaders/CWorldShadowShader.hpp"
#include "Graphics/Shaders/CParticleSwooshShaders.hpp"
#include "Audio/CStreamAudioManager.hpp"
#include "CGBASupport.hpp"

#include "CGameHintInfo.hpp"
#include "Particle/CParticleDataFactory.hpp"
#include "Particle/CGenDescription.hpp"
#include "Particle/CElectricDescription.hpp"
#include "Particle/CSwooshDescription.hpp"
#include "Particle/CParticleElectricDataFactory.hpp"
#include "Particle/CParticleSwooshDataFactory.hpp"
#include "Particle/CWeaponDescription.hpp"
#include "Particle/CProjectileWeaponDataFactory.hpp"
#include "Particle/CDecalDataFactory.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CRasterFont.hpp"
#include "GuiSys/CStringTable.hpp"
#include "Graphics/CModel.hpp"
#include "Graphics/CTexture.hpp"
#include "Character/CCharLayoutInfo.hpp"
#include "Character/CSkinRules.hpp"
#include "Character/CAnimCharacterSet.hpp"
#include "Character/CAllFormatsAnimSource.hpp"
#include "Character/CAnimPOIData.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "Collision/CCollisionResponseData.hpp"
#include "CSaveWorld.hpp"
#include "AutoMapper/CMapWorld.hpp"
#include "AutoMapper/CMapArea.hpp"
#include "AutoMapper/CMapUniverse.hpp"
#include "CScannableObjectInfo.hpp"
#include "Audio/CAudioGroupSet.hpp"
#include "Audio/CSfxManager.hpp"
#include "Audio/CMidiManager.hpp"
#include "CDependencyGroup.hpp"
#include "MP1OriginalIDs.hpp"

namespace urde
{
URDE_DECL_SPECIALIZE_SHADER(CParticleSwooshShaders)
URDE_DECL_SPECIALIZE_SHADER(CThermalColdFilter)
URDE_DECL_SPECIALIZE_SHADER(CThermalHotFilter)
URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)
URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)
URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)
URDE_DECL_SPECIALIZE_SHADER(CFogVolumePlaneShader)
URDE_DECL_SPECIALIZE_SHADER(CFogVolumeFilter)
URDE_DECL_SPECIALIZE_SHADER(CEnergyBarShader)
URDE_DECL_SPECIALIZE_SHADER(CRadarPaintShader)
URDE_DECL_SPECIALIZE_SHADER(CMapSurfaceShader)
URDE_DECL_SPECIALIZE_SHADER(CPhazonSuitFilter)
URDE_DECL_SPECIALIZE_SHADER(CAABoxShader)
URDE_DECL_SPECIALIZE_SHADER(CWorldShadowShader)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilterAlpha)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTextSupportShader)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CScanLinesFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CRandomStaticFilter)

namespace MP1
{

CGameArchitectureSupport::CGameArchitectureSupport(CMain& parent,
                                                   boo::IAudioVoiceEngine* voiceEngine,
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
    g_InputGenerator = &x30_inputGenerator;

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
    x4_archQueue.Push(MakeMsg::CreateFrameBegin(EArchMsgTarget::Game, x78_gameFrameCount));
    x4_archQueue.Push(MakeMsg::CreateTimerTick(EArchMsgTarget::Game, 1.f / 60.f));
}

void CGameArchitectureSupport::Update()
{
    g_GameState->GetWorldTransitionManager()->TouchModels();
    x30_inputGenerator.Update(1 / 60.f, x4_archQueue);
    x4_archQueue.Push(MakeMsg::CreateFrameEnd(EArchMsgTarget::Game, x78_gameFrameCount));
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
        auto name = CAudioSys::SysGetGroupSetName(tag->id);
        CAudioSys::SysRemoveGroupFromAmuse(name);
        CAudioSys::SysUnloadAudioGroupSet(name);
    }

    x8c_pendingAudioGroups = std::vector<TToken<CAudioGroupSet>>();
    x88_audioLoadStatus = EAudioLoadStatus::Uninitialized;
}

void CGameArchitectureSupport::Draw()
{
    x58_ioWinManager.Draw();
    if (m_parent.x161_24_gameFrameDrawn)
    {
        ++x78_gameFrameCount;
        m_parent.x161_24_gameFrameDrawn = false;
    }
}

CGameArchitectureSupport::~CGameArchitectureSupport()
{
    x58_ioWinManager.RemoveAllIOWins();
    UnloadAudio();
    CSfxManager::Shutdown();
    CStreamAudioManager::Shutdown();
}

CMain::CMain(IFactory* resFactory, CSimplePool* resStore,
             boo::IGraphicsDataFactory* gfxFactory,
             boo::IGraphicsCommandQueue* cmdQ,
             const boo::ObjToken<boo::ITextureR>& spareTex)
: m_booSetter(gfxFactory, cmdQ, spareTex),
  x128_globalObjects(resFactory, resStore)
{
    xe4_gameplayResult = EGameplayResult::Playing;
    g_Main = this;
}

CMain::BooSetter::BooSetter(boo::IGraphicsDataFactory* factory,
                            boo::IGraphicsCommandQueue* cmdQ,
                            const boo::ObjToken<boo::ITextureR>& spareTex)
{
    CGraphics::InitializeBoo(factory, cmdQ, spareTex);
    TShader<CParticleSwooshShaders>::Initialize();
    TShader<CThermalColdFilter>::Initialize();
    TShader<CThermalHotFilter>::Initialize();
    TShader<CSpaceWarpFilter>::Initialize();
    TShader<CCameraBlurFilter>::Initialize();
    TShader<CXRayBlurFilter>::Initialize();
    TShader<CFogVolumePlaneShader>::Initialize();
    TShader<CFogVolumeFilter>::Initialize();
    TShader<CEnergyBarShader>::Initialize();
    TShader<CRadarPaintShader>::Initialize();
    TShader<CMapSurfaceShader>::Initialize();
    TShader<CPhazonSuitFilter>::Initialize();
    TShader<CAABoxShader>::Initialize();
    TShader<CWorldShadowShader>::Initialize();
    TMultiBlendShader<CColoredQuadFilter>::Initialize();
    TMultiBlendShader<CTexturedQuadFilter>::Initialize();
    TMultiBlendShader<CTexturedQuadFilterAlpha>::Initialize();
    TMultiBlendShader<CTextSupportShader>::Initialize();
    TMultiBlendShader<CScanLinesFilter>::Initialize();
    TMultiBlendShader<CRandomStaticFilter>::Initialize();
}

void CMain::RegisterResourceTweaks()
{
}

void CGameGlobalObjects::AddPaksAndFactories()
{
    CGraphics::SetViewPointMatrix(zeus::CTransform::Identity());
    CGraphics::SetModelMatrix(zeus::CTransform::Identity());
    if (CResLoader* loader = g_ResFactory->GetResLoader())
    {
        loader->AddPakFileAsync("Tweaks", false, false);
        loader->AddPakFileAsync("NoARAM", false, false);
        loader->AddPakFileAsync("AudioGrp", false, false);
        loader->AddPakFileAsync("MiscData", false, false);
        loader->AddPakFileAsync("SamusGun", true, false);
        loader->AddPakFileAsync("TestAnim", true, false);
        loader->AddPakFileAsync("SamGunFx", true, false);
        loader->AddPakFileAsync("MidiData", false, false);
        loader->AddPakFileAsync("GGuiSys", false, false);
        loader->AddPakFileAsync("!original_ids", false, false);
        loader->WaitForPakFileLoadingComplete();
    }

    if (CFactoryMgr* fmgr = g_ResFactory->GetFactoryMgr())
    {
        fmgr->AddFactory(FOURCC('TXTR'), FMemFactoryFunc(FTextureFactory));
        fmgr->AddFactory(FOURCC('PART'), FFactoryFunc(FParticleFactory));
        fmgr->AddFactory(FOURCC('FRME'), FFactoryFunc(RGuiFrameFactoryInGame));
        fmgr->AddFactory(FOURCC('FONT'), FFactoryFunc(FRasterFontFactory));
        fmgr->AddFactory(FOURCC('CMDL'), FMemFactoryFunc(FModelFactory));
        fmgr->AddFactory(FOURCC('CINF'), FFactoryFunc(FCharLayoutInfo));
        fmgr->AddFactory(FOURCC('CSKR'), FFactoryFunc(FSkinRulesFactory));
        fmgr->AddFactory(FOURCC('ANCS'), FFactoryFunc(FAnimCharacterSet));
        fmgr->AddFactory(FOURCC('ANIM'), FFactoryFunc(AnimSourceFactory));
        fmgr->AddFactory(FOURCC('EVNT'), FFactoryFunc(AnimPOIDataFactory));
        fmgr->AddFactory(FOURCC('DCLN'), FFactoryFunc(FCollidableOBBTreeGroupFactory));
        fmgr->AddFactory(FOURCC('DGRP'), FFactoryFunc(FDependencyGroupFactory));
        fmgr->AddFactory(FOURCC('AGSC'), FMemFactoryFunc(FAudioGroupSetDataFactory));
        fmgr->AddFactory(FOURCC('CSNG'), FFactoryFunc(FMidiDataFactory));
        fmgr->AddFactory(FOURCC('ATBL'), FFactoryFunc(FAudioTranslationTableFactory));
        fmgr->AddFactory(FOURCC('STRG'), FFactoryFunc(FStringTableFactory));
        fmgr->AddFactory(FOURCC('HINT'), FFactoryFunc(FHintFactory));
        fmgr->AddFactory(FOURCC('SAVW'), FFactoryFunc(FSaveWorldFactory));
        fmgr->AddFactory(FOURCC('MAPW'), FFactoryFunc(FMapWorldFactory));
        fmgr->AddFactory(FOURCC('OIDS'), FFactoryFunc(FMP1OriginalIDsFactory));
        fmgr->AddFactory(FOURCC('SCAN'), FFactoryFunc(FScannableObjectInfoFactory));
        fmgr->AddFactory(FOURCC('CRSC'), FFactoryFunc(FCollisionResponseDataFactory));
        fmgr->AddFactory(FOURCC('SWHC'), FFactoryFunc(FParticleSwooshDataFactory));
        fmgr->AddFactory(FOURCC('ELSC'), FFactoryFunc(FParticleElectricDataFactory));
        fmgr->AddFactory(FOURCC('WPSC'), FFactoryFunc(FProjectileWeaponDataFactory));
        fmgr->AddFactory(FOURCC('DPSC'), FFactoryFunc(FDecalDataFactory));
        fmgr->AddFactory(FOURCC('MAPA'), FFactoryFunc(FMapAreaFactory));
        fmgr->AddFactory(FOURCC('MAPU'), FFactoryFunc(FMapUniverseFactory));
    }
}

void CMain::AddWorldPaks()
{
    CResLoader* loader = g_ResFactory->GetResLoader();
    if (!loader)
        return;
    auto pakPrefix = g_tweakGame->GetWorldPrefix();
    for (int i=0 ; i<9 ; ++i)
    {
        std::string path(pakPrefix);
        if (i != 0)
            path += '0' + i;
        if (CDvdFile::FileExists((path + ".upak").c_str()))
            loader->AddPakFileAsync(path, false, true);
    }
    loader->WaitForPakFileLoadingComplete();
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

void CMain::MemoryCardInitializePump()
{
    if (!g_MemoryCardSys)
    {
        std::unique_ptr<CMemoryCardSys>& memSys = x128_globalObjects.x0_memoryCardSys;
        if (!memSys)
            memSys.reset(new CMemoryCardSys());
        if (memSys->InitializePump())
        {
            g_MemoryCardSys = memSys.get();
            g_GameState->InitializeMemoryStates();
        }
    }
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

void CMain::EnsureWorldPaksReady()
{
}

void CMain::EnsureWorldPakReady(CAssetId mlvl)
{
    /* TODO: Schedule resource list load for World Pak containing mlvl */
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
                 boo::IWindow* window,
                 boo::IAudioVoiceEngine* voiceEngine,
                 amuse::IBackendVoiceAllocator& backend)
{
    m_mainWindow = window;
    InitializeSubsystems(storeMgr);
    x128_globalObjects.PostInitialize();
    x70_tweaks.RegisterTweaks();
    x70_tweaks.RegisterResourceTweaks();
    AddWorldPaks();
    FillInAssetIDs();
    x164_archSupport.reset(new CGameArchitectureSupport(*this, voiceEngine, backend));
    g_archSupport = x164_archSupport.get();
    x164_archSupport->PreloadAudio();
    //g_TweakManager->ReadFromMemoryCard("AudioTweaks");

    //CStreamAudioManager::Start(false, "Audio/rui_samusL.dsp|Audio/rui_samusR.dsp", 0x7f, true, 1.f, 1.f);
}

static logvisor::Module WarmupLog("ShaderWarmup");

void CMain::WarmupShaders()
{
    if (m_warmupTags.size())
        return;

    m_needsWarmupClear = true;
    size_t modelCount = 0;
    g_ResFactory->EnumerateResources([&](const SObjectTag& tag)
    {
        if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('MREA'))
            ++modelCount;
        return true;
    });
    m_warmupTags.reserve(modelCount);

    std::unordered_set<SObjectTag> addedTags;
    addedTags.reserve(modelCount);

    g_ResFactory->EnumerateResources([&](const SObjectTag& tag)
    {
        if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('MREA'))
        {
            if (addedTags.find(tag) != addedTags.end())
                return true;
            addedTags.insert(tag);
            m_warmupTags.push_back(tag);
        }
        return true;
    });

    m_warmupIt = m_warmupTags.begin();

    WarmupLog.report(logvisor::Info, "Began warmup of %" PRISize " objects", m_warmupTags.size());
}

bool CMain::Proc()
{
    // Warmup cycle overrides update
    if (m_warmupTags.size())
        return false;

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
    // Warmup cycle overrides draw
    if (m_warmupTags.size())
    {
        if (m_needsWarmupClear)
        {
            CGraphics::g_BooMainCommandQueue->clearTarget(true, true);
            m_needsWarmupClear = false;
        }
        auto startTime = std::chrono::steady_clock::now();
        while (m_warmupIt != m_warmupTags.end())
        {
            WarmupLog.report(logvisor::Info, "[%d / %d] Warming %.4s %08X",
                             int(m_warmupIt - m_warmupTags.begin() + 1), int(m_warmupTags.size()),
                             m_warmupIt->type.getChars(), m_warmupIt->id.Value());

            if (m_warmupIt->type == FOURCC('CMDL'))
                CModel::WarmupShaders(*m_warmupIt);
            else if (m_warmupIt->type == FOURCC('MREA'))
                CGameArea::WarmupShaders(*m_warmupIt);
            ++m_warmupIt;

            // Approximately 3/4 frame of warmups
            auto curTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(curTime - startTime).count() > 12)
                break;
        }
        if (m_warmupIt == m_warmupTags.end())
        {
            m_warmupTags = std::vector<SObjectTag>();
            WarmupLog.report(logvisor::Info, "Finished warmup");
        }
        return;
    }

    CGraphics::g_BooMainCommandQueue->clearTarget(true, true);
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
    CModelShaders::Shutdown();
    CMappableObject::Shutdown();
}

void CMain::Shutdown()
{
    x164_archSupport.reset();
    ShutdownSubsystems();
    TShader<CParticleSwooshShaders>::Shutdown();
    TShader<CThermalColdFilter>::Shutdown();
    TShader<CThermalHotFilter>::Shutdown();
    TShader<CSpaceWarpFilter>::Shutdown();
    TShader<CCameraBlurFilter>::Shutdown();
    TShader<CXRayBlurFilter>::Shutdown();
    TShader<CFogVolumePlaneShader>::Shutdown();
    TShader<CFogVolumeFilter>::Shutdown();
    TShader<CEnergyBarShader>::Shutdown();
    TShader<CRadarPaintShader>::Shutdown();
    TShader<CMapSurfaceShader>::Shutdown();
    TShader<CPhazonSuitFilter>::Shutdown();
    TShader<CAABoxShader>::Shutdown();
    TShader<CWorldShadowShader>::Shutdown();
    TMultiBlendShader<CColoredQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilterAlpha>::Shutdown();
    TMultiBlendShader<CTextSupportShader>::Shutdown();
    TMultiBlendShader<CScanLinesFilter>::Shutdown();
    TMultiBlendShader<CRandomStaticFilter>::Shutdown();
    CFluidPlaneShader::Shutdown();
}

boo::IWindow* CMain::GetMainWindow() const
{
    return m_mainWindow;
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
