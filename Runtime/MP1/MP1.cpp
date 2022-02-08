#include "Runtime/MP1/MP1.hpp"

#include <array>

#include "NESEmulator/CNESShader.hpp"

#include "Runtime/Graphics/Shaders/CAABoxShader.hpp"
#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Runtime/Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CColoredStripShader.hpp"
#include "Runtime/Graphics/Shaders/CEnergyBarShader.hpp"
#include "Runtime/Graphics/Shaders/CEnvFxShaders.hpp"
#include "Runtime/Graphics/Shaders/CFluidPlaneShader.hpp"
#include "Runtime/Graphics/Shaders/CMapSurfaceShader.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"
#include "Runtime/Graphics/Shaders/CParticleSwooshShaders.hpp"
#include "Runtime/Graphics/Shaders/CPhazonSuitFilter.hpp"
#include "Runtime/Graphics/Shaders/CRadarPaintShader.hpp"
#include "Runtime/Graphics/Shaders/CRandomStaticFilter.hpp"
#include "Runtime/Graphics/Shaders/CScanLinesFilter.hpp"
#include "Runtime/Graphics/Shaders/CSpaceWarpFilter.hpp"
#include "Runtime/Graphics/Shaders/CTextSupportShader.hpp"
#include "Runtime/Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Runtime/Graphics/Shaders/CThermalColdFilter.hpp"
#include "Runtime/Graphics/Shaders/CThermalHotFilter.hpp"
#include "Runtime/Graphics/Shaders/CWorldShadowShader.hpp"
#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CGameHintInfo.hpp"
#include "Runtime/CWorldSaveGameInfo.hpp"
#include "Runtime/CScannableObjectInfo.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/CStopwatch.hpp"
#include "Runtime/CTextureCache.hpp"
#include "Runtime/Audio/CAudioGroupSet.hpp"
#include "Runtime/Audio/CMidiManager.hpp"
#include "Runtime/Audio/CSfxManager.hpp"
#include "Runtime/Audio/CStreamAudioManager.hpp"
#include "Runtime/AutoMapper/CMapArea.hpp"
#include "Runtime/AutoMapper/CMapUniverse.hpp"
#include "Runtime/AutoMapper/CMapWorld.hpp"
#include "Runtime/Character/CAllFormatsAnimSource.hpp"
#include "Runtime/Character/CAnimCharacterSet.hpp"
#include "Runtime/Character/CAnimPOIData.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"
#include "Runtime/Collision/CCollisionResponseData.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/GuiSys/CGuiFrame.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/MP1/CGBASupport.hpp"
#include "Runtime/Particle/CDecalDataFactory.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CParticleElectricDataFactory.hpp"
#include "Runtime/Particle/CParticleSwooshDataFactory.hpp"
#include "Runtime/Particle/CProjectileWeaponDataFactory.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CStateMachine.hpp"
#include "Runtime/World/CScriptMazeNode.hpp"

#include <DataSpec/DNAMP1/SFX/Misc.h>
#include <DataSpec/DNAMP1/SFX/MiscSamus.h>
#include <DataSpec/DNAMP1/SFX/UI.h>
#include <DataSpec/DNAMP1/SFX/Weapons.h>
#include <DataSpec/DNAMP1/SFX/ZZZ.h>

#include "Runtime/MP1/CCredits.hpp"

#ifdef ENABLE_DISCORD
#include <discord_rpc.h>
#endif

namespace metaforce::MP1 {
namespace {
struct AudioGroupInfo {
  const char* name;
  u32 id;
};

constexpr std::array<AudioGroupInfo, 5> StaticAudioGroups{{
    {"Misc_AGSC", GRPMisc},
    {"MiscSamus_AGSC", GRPMiscSamus},
    {"UI_AGSC", GRPUI},
    {"Weapons_AGSC", GRPWeapons},
    {"ZZZ_AGSC", GRPZZZ},
}};
} // Anonymous namespace

CGameArchitectureSupport::CGameArchitectureSupport(CMain& parent, boo::IAudioVoiceEngine* voiceEngine,
                                                   amuse::IBackendVoiceAllocator& backend)
: m_parent(parent)
, x0_audioSys(voiceEngine, backend, 0, 0, 0, 0, 0)
, x30_inputGenerator(g_tweakPlayer->GetLeftLogicalThreshold(), g_tweakPlayer->GetRightLogicalThreshold())
, x44_guiSys(*g_ResFactory, *g_SimplePool, CGuiSys::EUsageMode::Zero) {
  auto* m = static_cast<CMain*>(g_Main);

//  x30_inputGenerator.startScanning();
  g_InputGenerator = &x30_inputGenerator;

  CAudioSys::SysSetVolume(0x7f);
  CAudioSys::SetDefaultVolumeScale(0x75);
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
  CStreamAudioManager::Initialize();
  CStreamAudioManager::SetMusicVolume(0x7f);
  m->ResetGameState();

  if (!g_tweakGame->GetSplashScreensDisabled()) {
    std::shared_ptr<CIOWin> splash = std::make_shared<CSplashScreen>(CSplashScreen::ESplashScreen::Nintendo);
    x58_ioWinManager.AddIOWin(splash, 1000, 10000);
  }

  std::shared_ptr<CIOWin> mf = std::make_shared<CMainFlow>();
  x58_ioWinManager.AddIOWin(mf, 0, 0);

  std::shared_ptr<CIOWin> console = std::make_shared<CConsoleOutputWindow>(8, 5.f, 0.75f);
  x58_ioWinManager.AddIOWin(console, 100, 0);

  std::shared_ptr<CIOWin> audState = std::make_shared<CAudioStateWin>();
  x58_ioWinManager.AddIOWin(audState, 100, -1);

  std::shared_ptr<CIOWin> errWin = std::make_shared<CErrorOutputWindow>(false);
  x58_ioWinManager.AddIOWin(errWin, 10000, 100000);

  g_GuiSys = &x44_guiSys;
  g_GameState->GameOptions().EnsureSettings();
}

void CGameArchitectureSupport::UpdateTicks(float dt) {
  x4_archQueue.Push(MakeMsg::CreateFrameBegin(EArchMsgTarget::Game, x78_gameFrameCount));
  x4_archQueue.Push(MakeMsg::CreateTimerTick(EArchMsgTarget::Game, dt));
}

void CGameArchitectureSupport::Update(float dt) {
  g_GameState->GetWorldTransitionManager()->TouchModels();
  x30_inputGenerator.Update(dt, x4_archQueue);
  x4_archQueue.Push(MakeMsg::CreateFrameEnd(EArchMsgTarget::Game, x78_gameFrameCount));
  x58_ioWinManager.PumpMessages(x4_archQueue);
}

bool CGameArchitectureSupport::LoadAudio() {
  if (x88_audioLoadStatus == EAudioLoadStatus::Loaded) {
    return true;
  }

  for (int i = 0; i < 5; ++i) {
    TToken<CAudioGroupSet>& tok = x8c_pendingAudioGroups[i];
    if (tok.IsLocked()) {
      if (tok.IsLoaded()) {
        CAudioGroupSet* set = tok.GetObj();
        if (!CAudioSys::SysIsGroupSetLoaded(set->GetName())) {
          CAudioSys::SysLoadGroupSet(tok, set->GetName(), tok.GetObjectTag()->id);
          CAudioSys::SysAddGroupIntoAmuse(set->GetName());
        }
      } else {
        return false;
      }
    } else {
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

void CGameArchitectureSupport::PreloadAudio() {
  if (x88_audioLoadStatus != EAudioLoadStatus::Uninitialized) {
    return;
  }

  x8c_pendingAudioGroups.clear();
  x8c_pendingAudioGroups.reserve(5);

  for (size_t i = 0; i < StaticAudioGroups.size(); ++i) {
    const AudioGroupInfo& info = StaticAudioGroups[i];
    CToken grp = g_SimplePool->GetObj(info.name);

    // Lock first group in sequence
    if (i == 0) {
      grp.Lock();
    }

    x8c_pendingAudioGroups.emplace_back(std::move(grp));
  }

  x88_audioLoadStatus = EAudioLoadStatus::Loading;
}

void CGameArchitectureSupport::UnloadAudio() {
  for (const AudioGroupInfo& info : StaticAudioGroups) {
    const SObjectTag* tag = g_ResFactory->GetResourceIdByName(info.name);
    auto name = CAudioSys::SysGetGroupSetName(tag->id);
    CAudioSys::SysRemoveGroupFromAmuse(name);
    CAudioSys::SysUnloadAudioGroupSet(name);
  }

  x8c_pendingAudioGroups = std::vector<TToken<CAudioGroupSet>>();
  x88_audioLoadStatus = EAudioLoadStatus::Uninitialized;
}

void CGameArchitectureSupport::Draw() {
  x58_ioWinManager.Draw();
  if (m_parent.x161_24_gameFrameDrawn) {
    ++x78_gameFrameCount;
    m_parent.x161_24_gameFrameDrawn = false;
  }
}

CGameArchitectureSupport::~CGameArchitectureSupport() {
  x58_ioWinManager.RemoveAllIOWins();
  UnloadAudio();
  CSfxManager::Shutdown();
  CStreamAudioManager::Shutdown();
}

void CGameArchitectureSupport::charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat) {
  x30_inputGenerator.charKeyDown(charCode, mods, isRepeat);
//  m_parent.m_console->handleCharCode(charCode, mods, isRepeat);
}

void CGameArchitectureSupport::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat) {
  x30_inputGenerator.specialKeyDown(key, mods, isRepeat);
//  m_parent.m_console->handleSpecialKeyDown(key, mods, isRepeat);
}

void CGameArchitectureSupport::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods) {
  x30_inputGenerator.specialKeyUp(key, mods);
//  m_parent.m_console->handleSpecialKeyUp(key, mods);
}

CMain::CMain(IFactory* resFactory, CSimplePool* resStore)
: m_booSetter()
, xe4_gameplayResult(EGameplayResult::Playing)
, x128_globalObjects(std::make_unique<CGameGlobalObjects>(resFactory, resStore)) {
  g_Main = this;
}

CMain::BooSetter::BooSetter() {
//  CGraphics::InitializeBoo();
  CParticleSwooshShaders::Initialize();
  CThermalColdFilter::Initialize();
  CThermalHotFilter::Initialize();
  CSpaceWarpFilter::Initialize();
  CCameraBlurFilter::Initialize();
  CXRayBlurFilter::Initialize();
//  CFogVolumePlaneShader::Initialize();
  CFogVolumeFilter::Initialize();
  CEnergyBarShader::Initialize();
  CRadarPaintShader::Initialize();
  CMapSurfaceShader::Initialize();
  CPhazonSuitFilter::Initialize();
  CAABoxShader::Initialize();
  CWorldShadowShader::Initialize();
  CColoredQuadFilter::Initialize();
  CColoredStripShader::Initialize();
  CTexturedQuadFilter::Initialize();
  CTexturedQuadFilterAlpha::Initialize();
  CTextSupportShader::Initialize();
  CScanLinesFilter::Initialize();
  CRandomStaticFilter::Initialize();
  CEnvFxShaders::Initialize();
  CNESShader::Initialize();
  CMoviePlayer::Initialize();
}

void CMain::RegisterResourceTweaks() {}

void CGameGlobalObjects::AddPaksAndFactories() {
  CGraphics::SetViewPointMatrix(zeus::CTransform());
  CGraphics::SetModelMatrix(zeus::CTransform());
  if (CResLoader* loader = g_ResFactory->GetResLoader()) {
    loader->AddPakFileAsync("Tweaks", false, false);
    loader->AddPakFileAsync("NoARAM", false, false);
    loader->AddPakFileAsync("AudioGrp", false, false);
    loader->AddPakFileAsync("MiscData", false, false);
    loader->AddPakFileAsync("SamusGun", true, false);
    loader->AddPakFileAsync("TestAnim", true, false);
    loader->AddPakFileAsync("SamGunFx", true, false);
    loader->AddPakFileAsync("MidiData", false, false);
    loader->AddPakFileAsync("GGuiSys", false, false);
    loader->WaitForPakFileLoadingComplete();
  }

  if (CFactoryMgr* fmgr = g_ResFactory->GetFactoryMgr()) {
    fmgr->AddFactory(FOURCC('TXTR'), FMemFactoryFunc(FTextureFactory));
    fmgr->AddFactory(FOURCC('PART'), FFactoryFunc(FParticleFactory));
    fmgr->AddFactory(FOURCC('FRME'), FFactoryFunc(RGuiFrameFactoryInGame));
    fmgr->AddFactory(FOURCC('FONT'), FFactoryFunc(FRasterFontFactory));
    fmgr->AddFactory(FOURCC('CMDL'), FMemFactoryFunc(FPCModelFactory));
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
    fmgr->AddFactory(FOURCC('SAVW'), FFactoryFunc(FWorldSaveGameInfoFactory));
    fmgr->AddFactory(FOURCC('MAPW'), FFactoryFunc(FMapWorldFactory));
    fmgr->AddFactory(FOURCC('SCAN'), FFactoryFunc(FScannableObjectInfoFactory));
    fmgr->AddFactory(FOURCC('CRSC'), FFactoryFunc(FCollisionResponseDataFactory));
    fmgr->AddFactory(FOURCC('SWHC'), FFactoryFunc(FParticleSwooshDataFactory));
    fmgr->AddFactory(FOURCC('ELSC'), FFactoryFunc(FParticleElectricDataFactory));
    fmgr->AddFactory(FOURCC('WPSC'), FFactoryFunc(FProjectileWeaponDataFactory));
    fmgr->AddFactory(FOURCC('DPSC'), FFactoryFunc(FDecalDataFactory));
    fmgr->AddFactory(FOURCC('MAPA'), FFactoryFunc(FMapAreaFactory));
    fmgr->AddFactory(FOURCC('MAPU'), FFactoryFunc(FMapUniverseFactory));
    fmgr->AddFactory(FOURCC('AFSM'), FFactoryFunc(FAiFiniteStateMachineFactory));
    fmgr->AddFactory(FOURCC('PATH'), FMemFactoryFunc(FPathFindAreaFactory));
    fmgr->AddFactory(FOURCC('TMET'), FFactoryFunc(FTextureCacheFactory));
  }
}

CGameGlobalObjects::~CGameGlobalObjects() {
  g_ResFactory = nullptr;
  g_SimplePool = nullptr;
  g_CharFactoryBuilder = nullptr;
  g_AiFuncMap = nullptr;
  g_GameState = nullptr;
  g_TweakManager = nullptr;
}
void CGameGlobalObjects::PostInitialize() {
  AddPaksAndFactories();
  LoadTextureCache();
  LoadStringTable();
  m_renderer.reset(AllocateRenderer(*xcc_simplePool, *x4_resFactory));
  CEnvFxManager::Initialize();
  CScriptMazeNode::LoadMazeSeeds();
}

void CMain::AddWorldPaks() {
  CResLoader* loader = g_ResFactory->GetResLoader();
  if (loader == nullptr) {
    return;
  }

  auto pakPrefix = g_tweakGame->GetWorldPrefix();
  for (int i = 0; i < 10; ++i) {
    std::string path(pakPrefix);

    if (i != 0) {
      path += '0' + char(i);
    }

    if (CDvdFile::FileExists(path + ".pak")) {
      loader->AddPakFileAsync(path, false, true);
    }
  }
  loader->WaitForPakFileLoadingComplete();
}

void CMain::AddOverridePaks() {
  CResLoader* loader = g_ResFactory->GetResLoader();
  if (loader == nullptr) {
    return;
  }

  /* Inversely load each pak starting at 999, to ensure proper priority order
   * the higher the number the higer the priority, e.g: Override0 has less priority than Override1 etc.
   */
  for (size_t i = 9; i > 0; --i) {
    const std::string path = fmt::format(FMT_STRING("Override{}"), i);
    if (CDvdFile::FileExists(path + ".pak")) {
      loader->AddPakFileAsync(path, false, false, true);
    }
  }
  /* Make sure all Override paks are ready before attempting to load URDE.pak */
  loader->WaitForPakFileLoadingComplete();

  /* Load Trilogy PAKs */
  if (CDvdFile::FileExists("RS5.pak")) {
    loader->AddPakFile("RS5", false, false, true);
  }
  if (CDvdFile::FileExists("Strings.pak")) {
    loader->AddPakFile("Strings", false, false, true);
  }

  /* Attempt to load URDE.pak
   * NOTE(phil): Should we fatal here if it's not found?
   */
  if (CDvdFile::FileExists("URDE.pak")) {
    loader->AddPakFile("URDE", false, false, true);
  }
}

void CMain::ResetGameState() {
  CPersistentOptions sysOpts = g_GameState->SystemOptions();
  CGameOptions gameOpts = g_GameState->GameOptions();
  x128_globalObjects->ResetGameState();
  g_GameState->ImportPersistentOptions(sysOpts);
  g_GameState->SetGameOptions(gameOpts);
  g_GameState->GetPlayerState()->SetIsFusionEnabled(g_GameState->SystemOptions().GetPlayerFusionSuitActive());
}

void CMain::InitializeSubsystems() {
  CBasics::Initialize();
  CModelShaders::Initialize();
  CLineRenderer::Initialize();
  CElementGen::Initialize();
  CAnimData::InitializeCache();
  CDecalManager::Initialize();
  CGBASupport::Initialize();
  CPatterned::Initialize();
//  CGraphics::g_BooFactory->waitUntilShadersReady();
}

void CMain::MemoryCardInitializePump() {
  if (g_MemoryCardSys != nullptr) {
    return;
  }

  std::unique_ptr<CMemoryCardSys>& memSys = x128_globalObjects->x0_memoryCardSys;
  if (!memSys) {
    memSys = std::make_unique<CMemoryCardSys>();
  }
  if (memSys->InitializePump()) {
    g_MemoryCardSys = memSys.get();
    g_GameState->InitializeMemoryStates();
  }
}

void CMain::FillInAssetIDs() {
  if (const SObjectTag* tag = g_ResFactory->GetResourceIdByName(g_tweakGame->GetDefaultRoom())) {
    g_GameState->SetCurrentWorldId(tag->id);
  }
}

bool CMain::LoadAudio() {
  if (x164_archSupport) {
    return x164_archSupport->LoadAudio();
  }
  return true;
}

void CMain::EnsureWorldPaksReady() {}

void CMain::EnsureWorldPakReady(CAssetId mlvl) { /* TODO: Schedule resource list load for World Pak containing mlvl */
}

void CMain::StreamNewGameState(CBitStreamReader& r, u32 idx) {
  bool fusionBackup = g_GameState->SystemOptions().GetPlayerFusionSuitActive();
  x128_globalObjects->x134_gameState = std::make_unique<CGameState>(r, idx);
  g_GameState = x128_globalObjects->x134_gameState.get();
  g_GameState->SystemOptions().SetPlayerFusionSuitActive(fusionBackup);
  g_GameState->GetPlayerState()->SetIsFusionEnabled(fusionBackup);
  g_GameState->HintOptions().SetNextHintTime();
}

void CMain::RefreshGameState() {
  CPersistentOptions sysOpts = g_GameState->SystemOptions();
  u64 cardSerial = g_GameState->GetCardSerial();
  std::vector<u8> saveData = g_GameState->BackupBuf();
  CGameOptions gameOpts = g_GameState->GameOptions();
  CBitStreamReader r(saveData.data(), saveData.size());
  x128_globalObjects->StreamInGameState(r, g_GameState->GetFileIdx());
  g_GameState->SetPersistentOptions(sysOpts);
  g_GameState->SetGameOptions(gameOpts);
  g_GameState->GameOptions().EnsureSettings();
  g_GameState->SetCardSerial(cardSerial);
  g_GameState->GetPlayerState()->SetIsFusionEnabled(g_GameState->SystemOptions().GetPlayerFusionSuitActive());
}

static logvisor::Module DiscordLog("Discord");
static logvisor::Module MainLog("MP1::CMain");
static const char* DISCORD_APPLICATION_ID = "402571593815031819";
static int64_t DiscordStartTime;
static CAssetId DiscordWorldSTRG;
static TLockedToken<CStringTable> DiscordWorldSTRGObj;
static std::string DiscordWorldName;
static u32 DiscordItemPercent = 0xffffffff;
static std::string DiscordState;

void CMain::InitializeDiscord() {
#ifdef ENABLE_DISCORD
  DiscordStartTime = std::time(nullptr);
  DiscordEventHandlers handlers = {};
  handlers.ready = HandleDiscordReady;
  handlers.disconnected = HandleDiscordDisconnected;
  handlers.errored = HandleDiscordErrored;
  Discord_Initialize(DISCORD_APPLICATION_ID, &handlers, 1, nullptr);
#endif
}

void CMain::ShutdownDiscord() {
#ifdef ENABLE_DISCORD
  DiscordWorldSTRGObj = TLockedToken<CStringTable>();
  Discord_Shutdown();
#endif
}

void CMain::UpdateDiscordPresence(CAssetId worldSTRG) {
#ifdef ENABLE_DISCORD
  bool updated = false;

  if (worldSTRG != DiscordWorldSTRG) {
    DiscordWorldSTRG = worldSTRG;
    DiscordWorldSTRGObj = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), worldSTRG});
  }
  if (DiscordWorldSTRGObj.IsLoaded()) {
    DiscordWorldName = hecl::Char16ToUTF8(DiscordWorldSTRGObj->GetString(0));
    DiscordWorldSTRGObj = TLockedToken<CStringTable>();
    updated = true;
  }

  if (g_GameState != nullptr) {
    if (const CPlayerState* pState = g_GameState->GetPlayerState().get()) {
      const u32 itemPercent = pState->CalculateItemCollectionRate() * 100 / pState->GetPickupTotal();
      if (DiscordItemPercent != itemPercent) {
        DiscordItemPercent = itemPercent;
        DiscordState = fmt::format(FMT_STRING("{}%"), itemPercent);
        updated = true;
      }
    }
  }

  if (updated) {
    DiscordRichPresence discordPresence = {};
    discordPresence.state = DiscordState.c_str();
    discordPresence.details = DiscordWorldName.c_str();
    discordPresence.largeImageKey = "default";
    discordPresence.startTimestamp = DiscordStartTime;
    Discord_UpdatePresence(&discordPresence);
  }
#endif
}

void CMain::HandleDiscordReady(const DiscordUser* request) {
  DiscordLog.report(logvisor::Info, FMT_STRING("Discord Ready"));
}

void CMain::HandleDiscordDisconnected(int errorCode, const char* message) {
  DiscordLog.report(logvisor::Warning, FMT_STRING("Discord Disconnected: {}"), message);
}

void CMain::HandleDiscordErrored(int errorCode, const char* message) {
  DiscordLog.report(logvisor::Error, FMT_STRING("Discord Error: {}"), message);
}

void CMain::Init(const hecl::Runtime::FileStoreManager& storeMgr, hecl::CVarManager* cvarMgr,
                 boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend) {
  InitializeDiscord();
  m_cvarMgr = cvarMgr;
  m_cvarCommons = std::make_unique<hecl::CVarCommons>(*m_cvarMgr);

  bool loadedVersion = false;
  if (CDvdFile::FileExists("version.yaml")) {
    CDvdFile file("version.yaml");
    if (file) {
      std::unique_ptr<u8[]> buf = std::make_unique<u8[]>(file.Length());
      u32 readLen = file.SyncRead(buf.get(), file.Length());
      if (readLen == file.Length()) {
        CMemoryInStream memoryInStream(buf.get(), file.Length());
        athena::io::FromYAMLStream(m_version, memoryInStream);
        loadedVersion = true;
        MainLog.report(logvisor::Level::Info, FMT_STRING("Loaded version info"));
      }
    }
  } else if (CDvdFile::FileExists("default.dol")) {
    CDvdFile file("default.dol");
    if (file) {
      std::unique_ptr<u8[]> buf = std::make_unique<u8[]>(file.Length());
      u32 readLen = file.SyncRead(buf.get(), file.Length());
      const char* buildInfo =
          static_cast<char*>(memmem(buf.get(), readLen, "MetroidBuildInfo", 16)) + 19;
      if (buildInfo != nullptr) {
        // TODO
        m_version = DataSpec::MetaforceVersionInfo{
            .version = std::string(buildInfo),
            .region = DataSpec::ERegion::NTSC_U,
            .game = DataSpec::EGame::MetroidPrime1,
            .isTrilogy = false,
        };
        loadedVersion = true;
        MainLog.report(logvisor::Level::Info, FMT_STRING("Loaded version info"));
      }
    }
  }

  InitializeSubsystems();
  AddOverridePaks();
  x128_globalObjects->PostInitialize();
  x70_tweaks.RegisterTweaks(m_cvarMgr);
  x70_tweaks.RegisterResourceTweaks(m_cvarMgr);
  AddWorldPaks();

  if (loadedVersion) {
    if (GetGame() != EGame::MetroidPrime1) {
      MainLog.report(logvisor::Level::Fatal,
                     FMT_STRING("Attempted to initialize URDE in MP1 mode with non-MP1 data!!!!"));
    }
    MainLog.report(logvisor::Level::Info, FMT_STRING("Loading data from Metroid Prime version {} from region {}{}"),
                   GetVersionString(), GetRegion(), IsTrilogy() ? " from trilogy" : "");
  } else {
    MainLog.report(logvisor::Level::Fatal, FMT_STRING("Unable to load version info"));
  }

  auto args = aurora::get_args();
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (*it == "--warp" && args.end() - it >= 3) {
      const char* worldIdxStr = (*(it + 1)).c_str();
      const char* areaIdxStr = (*(it + 2)).c_str();

      char* endptr = nullptr;
      m_warpWorldIdx = TAreaId(hecl::StrToUl(worldIdxStr, &endptr, 0));
      if (endptr == worldIdxStr) {
        m_warpWorldIdx = 0;
      }
      m_warpAreaId = TAreaId(hecl::StrToUl(areaIdxStr, &endptr, 0));
      if (endptr == areaIdxStr) {
        m_warpAreaId = 0;
      }

      bool found = false;
      for (const auto& pak : g_ResFactory->GetResLoader()->GetPaks()) {
        if (*(pak->GetPath().end() - 6) == '0' + m_warpWorldIdx) {
          found = true;
          break;
        }
      }

      if (!found) {
        m_warpWorldIdx = -1;
        break;
      }

      while (args.end() - it >= 4) {
        const char* layerStr = (*(it + 3)).c_str();
        if (!(layerStr[0] == '0' && layerStr[1] == 'x') &&
            (layerStr[0] == '0' || layerStr[0] == '1')) {
          for (const auto* cur = layerStr; *cur != '\0'; ++cur)
            if (*cur == '1')
              m_warpLayerBits |= u64(1) << (cur - layerStr);
        } else if (layerStr[0] == '0' && layerStr[1] == 'x') {
          m_warpMemoryRelays.emplace_back(TAreaId(hecl::StrToUl(layerStr + 2, nullptr, 16)));
        }
        ++it;
      }

      SetFlowState(EClientFlowStates::StateSetter);
      break;
    }
  }

  FillInAssetIDs();
  x164_archSupport = std::make_unique<CGameArchitectureSupport>(*this, voiceEngine, backend);
  g_archSupport = x164_archSupport.get();
  x164_archSupport->PreloadAudio();
  std::srand(static_cast<u32>(std::time(nullptr)));
  // g_TweakManager->ReadFromMemoryCard("AudioTweaks");
}

static logvisor::Module WarmupLog("ShaderWarmup");

void CMain::WarmupShaders() {
  if (!m_warmupTags.empty())
    return;

  m_needsWarmupClear = true;
  size_t modelCount = 0;
  g_ResFactory->EnumerateResources([&](const SObjectTag& tag) {
    if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('MREA')) {
      ++modelCount;
    }
    return true;
  });
  m_warmupTags.reserve(modelCount);

  std::unordered_set<SObjectTag> addedTags;
  addedTags.reserve(modelCount);

  g_ResFactory->EnumerateResources([&](const SObjectTag& tag) {
    if (tag.type == FOURCC('CMDL') || tag.type == FOURCC('MREA')) {
      if (addedTags.find(tag) != addedTags.end()) {
        return true;
      }
      addedTags.insert(tag);
      m_warmupTags.push_back(tag);
    }
    return true;
  });

  m_warmupIt = m_warmupTags.begin();

  WarmupLog.report(logvisor::Info, FMT_STRING("Began warmup of {} objects"), m_warmupTags.size());
}

bool CMain::Proc(float dt) {
  CRandom16::ResetNumNextCalls();
  // Warmup cycle overrides update
  if (m_warmupTags.size())
    return false;
  if (!m_loadedPersistentResources) {
    x128_globalObjects->m_gameResFactory->LoadPersistentResources(*g_SimplePool);
    m_loadedPersistentResources = true;
  }

  if (!m_paused) {
    CGBASupport::GlobalPoll();
    x164_archSupport->UpdateTicks(dt);
    x164_archSupport->Update(dt);
    CSfxManager::Update(dt);
    CStreamAudioManager::Update(dt);
  }

  if (x164_archSupport->GetIOWinManager().IsEmpty() || CheckReset()) {
    CStreamAudioManager::StopAll();
    /*
    x164_archSupport.reset();
    g_archSupport = x164_archSupport.get();
    x164_archSupport->PreloadAudio();
    */
    x160_24_finished = true;
  }

#ifdef ENABLE_DISCORD
  Discord_RunCallbacks();
#endif

  return x160_24_finished;
}

void CMain::Draw() {
  // Warmup cycle overrides draw
  if (m_warmupTags.size()) {
    if (m_needsWarmupClear) {
//      CGraphics::g_BooMainCommandQueue->clearTarget(true, true);
      m_needsWarmupClear = false;
    }
    auto startTime = std::chrono::steady_clock::now();
    while (m_warmupIt != m_warmupTags.end()) {
      WarmupLog.report(logvisor::Info, FMT_STRING("[{} / {}] Warming {}"), int(m_warmupIt - m_warmupTags.begin() + 1),
                       int(m_warmupTags.size()), *m_warmupIt);

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
    if (m_warmupIt == m_warmupTags.end()) {
      m_warmupTags = std::vector<SObjectTag>();
      WarmupLog.report(logvisor::Info, FMT_STRING("Finished warmup"));
    }
    return;
  }

  x164_archSupport->Draw();
//  m_console->draw(CGraphics::g_BooMainCommandQueue);
}

void CMain::ShutdownSubsystems() {
  CMoviePlayer::Shutdown();
  CLineRenderer::Shutdown();
  CDecalManager::Shutdown();
  CElementGen::Shutdown();
  CAnimData::FreeCache();
  CMemoryCardSys::Shutdown();
  CModelShaders::Shutdown();
  CMappableObject::Shutdown();
}

void CMain::Shutdown() {
//  m_console->unregisterCommand("Give");
  x128_globalObjects->m_gameResFactory->UnloadPersistentResources();
  x164_archSupport.reset();
  ShutdownSubsystems();
  CParticleSwooshShaders::Shutdown();
  CThermalColdFilter::Shutdown();
  CThermalHotFilter::Shutdown();
  CSpaceWarpFilter::Shutdown();
  CCameraBlurFilter::Shutdown();
  CXRayBlurFilter::Shutdown();
//  CFogVolumePlaneShader::Shutdown();
  CFogVolumeFilter::Shutdown();
  CEnergyBarShader::Shutdown();
  CRadarPaintShader::Shutdown();
  CMapSurfaceShader::Shutdown();
  CPhazonSuitFilter::Shutdown();
  CAABoxShader::Shutdown();
  CWorldShadowShader::Shutdown();
  CColoredQuadFilter::Shutdown();
  CColoredStripShader::Shutdown();
  CTexturedQuadFilter::Shutdown();
  CTexturedQuadFilterAlpha::Shutdown();
  CTextSupportShader::Shutdown();
  CScanLinesFilter::Shutdown();
  CRandomStaticFilter::Shutdown();
  CEnvFxShaders::Shutdown();
  CFluidPlaneShader::Shutdown();
  CFluidPlaneManager::RippleMapTex.reset();
  CNESShader::Shutdown();
  CBooModel::Shutdown();
//  CGraphics::ShutdownBoo();
  ShutdownDiscord();
}

#if 0
int CMain::RsMain(int argc, char** argv, boo::IAudioVoiceEngine* voiceEngine,
                  amuse::IBackendVoiceAllocator& backend) {
  // PPCSetFpIEEEMode();
  // uVar21 = OSGetTime();
  // LCEnable();
  x128_globalObjects = std::make_unique<CGameGlobalObjects>(nullptr, nullptr);
  xf0_.resize(4, 0.3f);
  x104_.resize(4, 0.2f);
  x118_ = 0.3f;
  x11c_ = 0.2f;
  InitializeSubsystems();
  x128_globalObjects->PostInitialize(); // COsContext*, CMemorySys*
  x70_tweaks.RegisterTweaks(m_cvarMgr);
  AddWorldPaks();

  std::string msg;
  if (!g_TweakManager->ReadFromMemoryCard("AudioTweaks"sv)) {
    msg = "Loaded audio tweaks from memory card\n"s;
  } else {
    msg = "FAILED to load audio tweaks from memory card\n";
  }

  FillInAssetIDs();
  x164_archSupport = std::make_unique<CGameArchitectureSupport>(*this, voiceEngine, backend);
  x164_archSupport->PreloadAudio();

  return 0;
}
#endif

} // namespace metaforce::MP1
