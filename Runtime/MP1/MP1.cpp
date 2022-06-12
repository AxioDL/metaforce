#include "Runtime/MP1/MP1.hpp"

#include <array>

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
#include "Runtime/Graphics/CFont.hpp"
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

#include "Runtime/MP1/CCredits.hpp"

#include <magic_enum.hpp>

#ifdef ENABLE_DISCORD
#include <discord_rpc.h>
#endif

#if _WIN32
inline void* memmem(const void* haystack, size_t hlen, const void* needle, size_t nlen) {
  int needle_first;
  const uint8_t* p = static_cast<const uint8_t*>(haystack);
  size_t plen = hlen;

  if (!nlen)
    return NULL;

  needle_first = *(unsigned char*)needle;

  while (plen >= nlen && (p = static_cast<const uint8_t*>(memchr(p, needle_first, plen - nlen + 1)))) {
    if (!memcmp(p, needle, nlen))
      return (void*)p;

    p++;
    plen = hlen - (p - static_cast<const uint8_t*>(haystack));
  }

  return NULL;
}
#endif

namespace metaforce::MP1 {
static logvisor::Module Log{"MP1"};

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
, x30_inputGenerator(/*osCtx, */ g_tweakPlayer->GetLeftLogicalThreshold(), g_tweakPlayer->GetRightLogicalThreshold())
, x44_guiSys(*g_ResFactory, *g_SimplePool, CGuiSys::EUsageMode::Zero) {
  auto* m = static_cast<CMain*>(g_Main);

  g_InputGenerator = &x30_inputGenerator;
  g_Controller = x30_inputGenerator.GetController();

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

void CGameArchitectureSupport::charKeyDown(uint8_t charCode, aurora::ModifierKey mods, bool isRepeat) {
  // x30_inputGenerator.charKeyDown(charCode, mods, isRepeat);
}

void CGameArchitectureSupport::specialKeyDown(aurora::SpecialKey key, aurora::ModifierKey mods, bool isRepeat) {
  // x30_inputGenerator.specialKeyDown(key, mods, isRepeat);
}

void CGameArchitectureSupport::specialKeyUp(aurora::SpecialKey key, aurora::ModifierKey mods) {
  // x30_inputGenerator.specialKeyUp(key, mods);
}

CMain::CMain(IFactory* resFactory, CSimplePool* resStore)
: xe4_gameplayResult(EGameplayResult::Playing)
, x128_globalObjects(std::make_unique<CGameGlobalObjects>(resFactory, resStore)) {
  g_Main = this;
}

CMain::~CMain() { g_Main = nullptr; }

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
    fmgr->AddFactory(FOURCC('TXTR'), FFactoryFunc(FTextureFactory));
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
  CElementGen::Initialize();
  CAnimData::InitializeCache();
  CDecalManager::Initialize();
  CGBASupport::Initialize();
  CPatterned::Initialize();
  // Metaforce additions
  CMoviePlayer::Initialize();
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

void CMain::StreamNewGameState(CInputStream& r, u32 idx) {
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
  CMemoryInStream r(saveData.data(), saveData.size(), CMemoryInStream::EOwnerShip::NotOwned);
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

std::string CMain::Init(const FileStoreManager& storeMgr, CVarManager* cvarMgr, boo::IAudioVoiceEngine* voiceEngine,
                        amuse::IBackendVoiceAllocator& backend) {
  m_cvarMgr = cvarMgr;

  {
    auto discInfo = CDvdFile::DiscInfo();
    if (discInfo.gameId[4] != '0' || discInfo.gameId[5] != '1') {
      return fmt::format(FMT_STRING("Unknown game ID {}"), std::string_view{discInfo.gameId.data(), 6});
    }
    if (strncmp(discInfo.gameId.data(), "GM8", 3) == 0) {
      m_version.game = EGame::MetroidPrime1;
      m_version.platform = EPlatform::GameCube;
    } else if (strncmp(discInfo.gameId.data(), "R3I", 3) == 0) {
      m_version.game = EGame::MetroidPrime1;
      m_version.platform = EPlatform::Wii;
    } else if (strncmp(discInfo.gameId.data(), "G2M", 3) == 0) {
      m_version.game = EGame::MetroidPrime2;
      m_version.platform = EPlatform::GameCube;
    } else if (strncmp(discInfo.gameId.data(), "R32", 3) == 0) {
      m_version.game = EGame::MetroidPrime2;
      m_version.platform = EPlatform::Wii;
    } else if (strncmp(discInfo.gameId.data(), "RM3", 3) == 0) {
      m_version.game = EGame::MetroidPrime3;
      m_version.platform = EPlatform::Wii;
    } else if (strncmp(discInfo.gameId.data(), "R3M", 3) == 0) {
      m_version.game = EGame::MetroidPrimeTrilogy;
      m_version.platform = EPlatform::Wii;
    } else {
      return fmt::format(FMT_STRING("Unknown game ID {}"), std::string_view{discInfo.gameId.data(), 6});
    }
    switch (discInfo.gameId[3]) {
    case 'E':
      if (m_version.game == EGame::MetroidPrime1 && discInfo.version == 48) {
        m_version.region = ERegion::KOR;
      } else {
        m_version.region = ERegion::USA;
      }
      break;
    case 'J':
      m_version.region = ERegion::JPN;
      break;
    case 'P':
      m_version.region = ERegion::PAL;
      break;
    default:
      return fmt::format(FMT_STRING("Unknown region {}"), discInfo.gameId[3]);
    }
    m_version.gameTitle = std::move(discInfo.gameTitle);
  }

  if (m_version.game != EGame::MetroidPrime1 && m_version.game != EGame::MetroidPrimeTrilogy) {
    return fmt::format(FMT_STRING("Unsupported game {}"), magic_enum::enum_name(m_version.game));
  }

  {
    auto dolFile = "default.dol"sv;
    if (m_version.game == EGame::MetroidPrimeTrilogy) {
      dolFile = "rs5mp1_p.dol"sv;
    } else if (m_version.platform == EPlatform::Wii) {
      dolFile = "rs5mp1jpn_p.dol"sv;
    }
    CDvdFile file(dolFile);
    if (!file) {
      return fmt::format(FMT_STRING("Failed to open {}"), dolFile);
    }
    std::unique_ptr<u8[]> buf = std::make_unique<u8[]>(file.Length());
    u32 readLen = file.SyncRead(buf.get(), file.Length());
    const char* buildInfo = static_cast<char*>(memmem(buf.get(), readLen, "MetroidBuildInfo", 16)) + 19;
    if (buildInfo == nullptr) {
      return fmt::format(FMT_STRING("Failed to locate MetroidBuildInfo"));
    }
    m_version.version = buildInfo;
  }
  MainLog.report(logvisor::Level::Info, FMT_STRING("Loading data from {} {} ({})"), GetGameTitle(),
                 magic_enum::enum_name(GetRegion()), GetVersionString());

  InitializeDiscord();
  if (m_version.game == EGame::MetroidPrimeTrilogy) {
    CDvdFile::SetRootDirectory("MP1");
  } else if (m_version.platform == EPlatform::Wii) {
    CDvdFile::SetRootDirectory("MP1JPN");
  }
  InitializeSubsystems();
  AddOverridePaks();
  x128_globalObjects->PostInitialize();
  x70_tweaks.RegisterTweaks(m_cvarMgr);
  x70_tweaks.RegisterResourceTweaks(m_cvarMgr);
  AddWorldPaks();

  auto args = aurora::get_args();
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (*it == "--warp" && args.end() - it >= 3) {
      const char* worldIdxStr = (*(it + 1)).c_str();
      const char* areaIdxStr = (*(it + 2)).c_str();

      char* endptr = nullptr;
      m_warpWorldIdx = TAreaId(strtoul(worldIdxStr, &endptr, 0));
      if (endptr == worldIdxStr) {
        m_warpWorldIdx = 0;
      }
      m_warpAreaId = TAreaId(strtoul(areaIdxStr, &endptr, 0));
      if (endptr == areaIdxStr) {
        m_warpAreaId = 0;
      }

      bool found = false;
      for (const auto& pak : g_ResFactory->GetResLoader()->GetPaks()) {
        if (*(pak->GetPath().end() - 5) == '0' + m_warpWorldIdx) {
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
        if (!(layerStr[0] == '0' && layerStr[1] == 'x') && (layerStr[0] == '0' || layerStr[0] == '1')) {
          for (const auto* cur = layerStr; *cur != '\0'; ++cur)
            if (*cur == '1')
              m_warpLayerBits |= u64(1) << (cur - layerStr);
        } else if (layerStr[0] == '0' && layerStr[1] == 'x') {
          m_warpMemoryRelays.emplace_back(TAreaId(strtoul(layerStr + 2, nullptr, 16)));
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
  std::srand(static_cast<u32>(CBasics::GetTime()));
  // g_TweakManager->ReadFromMemoryCard("AudioTweaks");
  return {};
}

bool CMain::Proc(float dt) {
  CRandom16::ResetNumNextCalls();
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

void CMain::Draw() { x164_archSupport->Draw(); }

void CMain::ShutdownSubsystems() {
  CDecalManager::Shutdown();
  CElementGen::Shutdown();
  CAnimData::FreeCache();
  CMemoryCardSys::Shutdown();
  CMappableObject::Shutdown();
  // Metaforce additions
  CMoviePlayer::Shutdown();
  CFont::Shutdown();
  CFluidPlaneManager::Shutdown();
}

void CMain::Shutdown() {
  x128_globalObjects->m_gameResFactory->UnloadPersistentResources();
  x164_archSupport.reset();
  ShutdownSubsystems();
  //  CBooModel::Shutdown();
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
