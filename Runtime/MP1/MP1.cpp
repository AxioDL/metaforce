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
#include "Runtime/CSaveWorld.hpp"
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
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CStateMachine.hpp"
#include "Runtime/World/CScriptMazeNode.hpp"

#include <DataSpec/DNAMP1/SFX/Misc.h>
#include <DataSpec/DNAMP1/SFX/MiscSamus.h>
#include <DataSpec/DNAMP1/SFX/UI.h>
#include <DataSpec/DNAMP1/SFX/Weapons.h>
#include <DataSpec/DNAMP1/SFX/ZZZ.h>

#include <discord_rpc.h>

namespace urde::MP1 {
namespace {
struct AudioGroupInfo {
  const char* name;
  u32 id;
};

constexpr std::array<AudioGroupInfo, 5> StaticAudioGroups{{
    {"Misc_AGSC", GRPmisc},
    {"MiscSamus_AGSC", GRPmiscSamus},
    {"UI_AGSC", GRPui},
    {"Weapons_AGSC", GRPweapons},
    {"ZZZ_AGSC", GRPzzz},
}};
} // Anonymous namespace

CGameArchitectureSupport::CGameArchitectureSupport(CMain& parent, boo::IAudioVoiceEngine* voiceEngine,
                                                   amuse::IBackendVoiceAllocator& backend)
: m_parent(parent)
, x0_audioSys(voiceEngine, backend, 0, 0, 0, 0, 0)
, x30_inputGenerator(g_tweakPlayer->GetLeftLogicalThreshold(), g_tweakPlayer->GetRightLogicalThreshold())
, x44_guiSys(*g_ResFactory, *g_SimplePool, CGuiSys::EUsageMode::Zero) {
  auto* m = static_cast<CMain*>(g_Main);

  x30_inputGenerator.startScanning();
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
  m_parent.m_console->handleCharCode(charCode, mods, isRepeat);
}

void CGameArchitectureSupport::specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat) {
  x30_inputGenerator.specialKeyDown(key, mods, isRepeat);
  m_parent.m_console->handleSpecialKeyDown(key, mods, isRepeat);
}

void CGameArchitectureSupport::specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods) {
  x30_inputGenerator.specialKeyUp(key, mods);
  m_parent.m_console->handleSpecialKeyUp(key, mods);
}

CMain::CMain(IFactory* resFactory, CSimplePool* resStore, boo::IGraphicsDataFactory* gfxFactory,
             boo::IGraphicsCommandQueue* cmdQ, const boo::ObjToken<boo::ITextureR>& spareTex)
: m_booSetter(gfxFactory, cmdQ, spareTex), x128_globalObjects(resFactory, resStore) {
  xe4_gameplayResult = EGameplayResult::Playing;
  g_Main = this;
}

CMain::BooSetter::BooSetter(boo::IGraphicsDataFactory* factory, boo::IGraphicsCommandQueue* cmdQ,
                            const boo::ObjToken<boo::ITextureR>& spareTex) {
  CGraphics::InitializeBoo(factory, cmdQ, spareTex);
  CParticleSwooshShaders::Initialize();
  CThermalColdFilter::Initialize();
  CThermalHotFilter::Initialize();
  CSpaceWarpFilter::Initialize();
  CCameraBlurFilter::Initialize();
  CXRayBlurFilter::Initialize();
  CFogVolumePlaneShader::Initialize();
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
  for (int i = 0; i < 9; ++i) {
    std::string path(pakPrefix);

    if (i != 0) {
      path += '0' + char(i);
    }

    if (CDvdFile::FileExists(path + ".upak")) {
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
  for (size_t i = 999; i > 0; --i) {
    const std::string path = fmt::format(FMT_STRING("Override{}"), i);
    if (CDvdFile::FileExists(path + ".upak")) {
      loader->AddPakFileAsync(path, false, false, true);
    }
  }
  /* Make sure all Override paks are ready before attempting to load URDE.upak */
  loader->WaitForPakFileLoadingComplete();

  /* Attempt to load URDE.upak
   * NOTE(phil): Should we fatal here if it's not found?
   */
  if (CDvdFile::FileExists("URDE.upak")) {
    loader->AddPakFile("URDE", false, false, true);
  }
}

void CMain::ResetGameState() {
  CPersistentOptions sysOpts = g_GameState->SystemOptions();
  CGameOptions gameOpts = g_GameState->GameOptions();
  x128_globalObjects.ResetGameState();
  g_GameState->ImportPersistentOptions(sysOpts);
  g_GameState->SetGameOptions(gameOpts);
  g_GameState->GetPlayerState()->SetIsFusionEnabled(g_GameState->SystemOptions().GetPlayerFusionSuitActive());
}

void CMain::InitializeSubsystems() {
  CBasics::Initialize();
  CModelShaders::Initialize();
  CMoviePlayer::Initialize();
  CLineRenderer::Initialize();
  CElementGen::Initialize();
  CAnimData::InitializeCache();
  CDecalManager::Initialize();
  CGBASupport::Initialize();
  CGraphics::g_BooFactory->waitUntilShadersReady();
}

void CMain::MemoryCardInitializePump() {
  if (g_MemoryCardSys != nullptr) {
    return;
  }

  std::unique_ptr<CMemoryCardSys>& memSys = x128_globalObjects.x0_memoryCardSys;
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

void CMain::Give(hecl::Console* console, const std::vector<std::string>& args) {
  if (args.empty() || (g_GameState == nullptr || !g_GameState->GetPlayerState())) {
    return;
  }

  std::string type = args[0];
  athena::utility::tolower(type);
  std::shared_ptr<CPlayerState> pState = g_GameState->GetPlayerState();
  if (type == "all") {
    for (u32 item = 0; item < u32(CPlayerState::EItemType::Max); ++item) {
      pState->ReInitalizePowerUp(CPlayerState::EItemType(item),
                                 CPlayerState::GetPowerUpMaxValue(CPlayerState::EItemType(item)));
      pState->IncrPickup(CPlayerState::EItemType(item),
                         CPlayerState::GetPowerUpMaxValue(CPlayerState::EItemType(item)));
    }
    pState->IncrPickup(CPlayerState::EItemType::HealthRefill, 99999);
  } else if (type == "map") {
    g_GameState->CurrentWorldState().MapWorldInfo()->SetMapStationUsed(true);
  } else {
    CPlayerState::EItemType eType = CPlayerState::ItemNameToType(type);
    if (eType == CPlayerState::EItemType::Invalid) {
      console->report(hecl::Console::Level::Info, FMT_STRING("Invalid item {}"), type);
      return;
    }
    if (eType == CPlayerState::EItemType::HealthRefill) {
      pState->IncrPickup(eType, 9999);
      console->report(hecl::Console::Level::Info,
                      FMT_STRING("Cheater....., Greatly increasing Metroid encounters, have fun!"));
      if (g_StateManager != nullptr) {
        g_StateManager->Player()->AsyncLoadSuit(*g_StateManager);
      }
      return;
    }

    s32 itemAmt = CPlayerState::GetPowerUpMaxValue(eType);
    if (args.size() == 2) {
      s32 itemMax = CPlayerState::GetPowerUpMaxValue(eType);
      itemAmt = s32(strtol(args[1].c_str(), nullptr, 10));
      itemAmt = zeus::clamp(-itemMax, itemAmt, itemMax);
    }

    u32 curCap = pState->GetItemCapacity(eType);
    if (itemAmt > 0 && curCap < u32(itemAmt)) {
      /* Handle special case with Missiles */
      if (eType == CPlayerState::EItemType::Missiles) {
        u32 tmp = ((u32(itemAmt) / 5) + (itemAmt % 5)) * 5;
        pState->ReInitalizePowerUp(eType, tmp);
      } else {
        pState->ReInitalizePowerUp(eType, itemAmt);
      }
    }

    if (itemAmt > 0) {
      pState->IncrPickup(eType, u32(itemAmt));
    } else {
      pState->DecrPickup(eType, zeus::clamp(0u, u32(abs(itemAmt)), pState->GetItemAmount(eType)));
    }
  }
  if (g_StateManager != nullptr) {
    g_StateManager->Player()->AsyncLoadSuit(*g_StateManager);
  }
  console->report(hecl::Console::Level::Info,
                  FMT_STRING("Cheater....., Greatly increasing Metroid encounters, have fun!"));
} // namespace MP1

void CMain::Remove(hecl::Console*, const std::vector<std::string>& args) {
  if (args.empty() || (g_GameState == nullptr || !g_GameState->GetPlayerState())) {
    return;
  }

  std::string type = args[0];
  athena::utility::tolower(type);
  std::shared_ptr<CPlayerState> pState = g_GameState->GetPlayerState();
  if (type == "all") {

  } else if (type == "map") {
    g_GameState->CurrentWorldState().MapWorldInfo()->SetMapStationUsed(false);
  } else {
    CPlayerState::EItemType eType = CPlayerState::ItemNameToType(type);
    if (eType != CPlayerState::EItemType::Invalid) {
      pState->ReInitalizePowerUp(eType, 0);
      if (g_StateManager != nullptr) {
        g_StateManager->Player()->AsyncLoadSuit(*g_StateManager);
      }
    }
  }
}

void CMain::God(hecl::Console* con, const std::vector<std::string>&) {
  if (g_GameState != nullptr && g_GameState->GetPlayerState()) {
    g_GameState->GetPlayerState()->SetCanTakeDamage(!g_GameState->GetPlayerState()->CanTakeDamage());
    if (!g_GameState->GetPlayerState()->CanTakeDamage()) {
      con->report(hecl::Console::Level::Info, FMT_STRING("God Mode Enabled"));
    } else {
      con->report(hecl::Console::Level::Info, FMT_STRING("God Mode Disabled"));
    }
  }
}

void CMain::Teleport(hecl::Console*, const std::vector<std::string>& args) {
  if (g_StateManager == nullptr || args.size() < 3) {
    return;
  }

  zeus::CVector3f loc;
  for (u32 i = 0; i < 3; ++i) {
    loc[i] = strtof(args[i].c_str(), nullptr);
  }

  zeus::CTransform xf = g_StateManager->Player()->GetTransform();
  xf.origin = loc;

  if (args.size() >= 6) {
    zeus::CVector3f angle;
    for (u32 i = 0; i < 3; ++i) {
      angle[i] = zeus::degToRad(strtof(args[i + 3].c_str(), nullptr));
    }
    xf.setRotation(zeus::CMatrix3f(zeus::CQuaternion(angle)));
  }
  g_StateManager->Player()->Teleport(xf, *g_StateManager, false);
}

void CMain::ListWorlds(hecl::Console* con, const std::vector<std::string>&) {

  if (g_ResFactory != nullptr && g_ResFactory->GetResLoader() != nullptr) {
    for (const auto& pak : g_ResFactory->GetResLoader()->GetPaks()) {
      if (pak->IsWorldPak()) {
        for (const auto& named : pak->GetNameList()) {
          if (named.second.type == SBIG('MLVL')) {
            con->report(hecl::Console::Level::Info, FMT_STRING("{} '{}'"), named.first, named.second.id);
          }
        }
      }
    }
  }
}

void CMain::Warp(hecl::Console* con, const std::vector<std::string>& args) {
  if (g_StateManager == nullptr) {
    return;
  }

  if (args.empty()) {
    return;
  }

  TAreaId aId = 0;
  std::string worldName;
  if (args.size() == 2) {
    worldName = args[0];
    athena::utility::tolower(worldName);
    aId = strtol(args[1].c_str(), nullptr, 10);
  } else {
    aId = strtol(args[0].c_str(), nullptr, 10);
  }

  if (!worldName.empty() && g_ResFactory != nullptr && g_ResFactory->GetResLoader() != nullptr) {
    bool found = false;

    for (const auto& pak : g_ResFactory->GetResLoader()->GetPaks()) {
      if (found) {
        break;
      }
      if (pak->IsWorldPak()) {
        for (const auto& named : pak->GetNameList()) {
          if (named.second.type == SBIG('MLVL')) {
            std::string name = named.first;
            athena::utility::tolower(name);
            if (name.find(worldName) != std::string::npos) {
              g_GameState->SetCurrentWorldId(named.second.id);
              found = true;
              break;
            }
          }
        }
      }
    }
  }

  g_GameState->GetWorldTransitionManager()->DisableTransition();

  if (aId >= g_GameState->CurrentWorldState().GetLayerState()->GetAreaCount()) {
    aId = 0;
  }

  g_GameState->CurrentWorldState().SetAreaId(aId);
  g_Main->SetFlowState(EFlowState::None);
  g_StateManager->SetWarping(true);
  g_StateManager->SetShouldQuitGame(true);
}

void CMain::StreamNewGameState(CBitStreamReader& r, u32 idx) {
  bool fusionBackup = g_GameState->SystemOptions().GetPlayerFusionSuitActive();
  x128_globalObjects.x134_gameState = std::make_unique<CGameState>(r, idx);
  g_GameState = x128_globalObjects.x134_gameState.get();
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
  x128_globalObjects.StreamInGameState(r, g_GameState->GetFileIdx());
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
  DiscordStartTime = std::time(nullptr);
  DiscordEventHandlers handlers = {};
  handlers.ready = HandleDiscordReady;
  handlers.disconnected = HandleDiscordDisconnected;
  handlers.errored = HandleDiscordErrored;
  Discord_Initialize(DISCORD_APPLICATION_ID, &handlers, 1, nullptr);
}

void CMain::ShutdownDiscord() {
  DiscordWorldSTRGObj = TLockedToken<CStringTable>();
  Discord_Shutdown();
}

void CMain::UpdateDiscordPresence(CAssetId worldSTRG) {
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
    if (CPlayerState* pState = g_GameState->GetPlayerState().get()) {
      u32 itemPercent = pState->CalculateItemCollectionRate() * 100 / pState->GetPickupTotal();
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

void CMain::Init(const hecl::Runtime::FileStoreManager& storeMgr, hecl::CVarManager* cvarMgr, boo::IWindow* window,
                 boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend) {
  InitializeDiscord();
  m_mainWindow = window;
  m_cvarMgr = cvarMgr;
  m_cvarCommons = std::make_unique<hecl::CVarCommons>(*m_cvarMgr);
  m_console = std::make_unique<hecl::Console>(m_cvarMgr);
  m_console->init(window);
  m_console->registerCommand(
      "Quit"sv, "Quits the game immediately"sv, ""sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { quit(console, args); });
  m_console->registerCommand(
      "Give"sv, "Gives the player the specified item, maxing it out"sv, ""sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { Give(console, args); },
      hecl::SConsoleCommand::ECommandFlags::Cheat);
  m_console->registerCommand(
      "Remove"sv, "Removes the specified item from the player"sv, ""sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { Remove(console, args); },
      hecl::SConsoleCommand::ECommandFlags::Cheat);
  m_console->registerCommand(
      "Teleport"sv, "Teleports the player to the specified coordinates in worldspace"sv, "x y z [dX dY dZ]"sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { Teleport(console, args); },
      (hecl::SConsoleCommand::ECommandFlags::Cheat | hecl::SConsoleCommand::ECommandFlags::Developer));
  m_console->registerCommand(
      "God"sv, "Disables damage given by enemies and objects"sv, ""sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { God(console, args); },
      hecl::SConsoleCommand::ECommandFlags::Cheat);
  m_console->registerCommand(
      "ListWorlds"sv, "Lists loaded worlds"sv, ""sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { ListWorlds(console, args); },
      hecl::SConsoleCommand::ECommandFlags::Normal);
  m_console->registerCommand(
      "Warp"sv, "Warps to a given area and world"sv, "[worldname] areaId"sv,
      [this](hecl::Console* console, const std::vector<std::string>& args) { Warp(console, args); },
      hecl::SConsoleCommand::ECommandFlags::Normal);

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
  }

  InitializeSubsystems();
  AddOverridePaks();
  x128_globalObjects.PostInitialize();
  x70_tweaks.RegisterTweaks(m_cvarMgr);
  x70_tweaks.RegisterResourceTweaks(m_cvarMgr);
  AddWorldPaks();

  if (loadedVersion) {
    if (GetGame() != EGame::MetroidPrime1) {
      MainLog.report(logvisor::Level::Fatal,
                     FMT_STRING("Attempted to initialize URDE in MP1 mode with non-MP1 data!!!!"));
    }
    hecl::SystemStringConv conv(GetVersionString());
    boo::SystemStringView versionView(conv.sys_str());
    MainLog.report(logvisor::Level::Info, FMT_STRING(_SYS_STR("Loading data from Metroid Prime version {} from region {}{}")),
                   versionView, boo::SystemChar(GetRegion()), IsTrilogy() ? _SYS_STR(" from trilogy") : _SYS_STR(""));
  } else {
    MainLog.report(logvisor::Level::Fatal, FMT_STRING("Unable to load version info"));
  }

  const auto& args = boo::APP->getArgs();
  for (auto it = args.begin(); it != args.end(); ++it) {
    if (*it == _SYS_STR("--warp") && args.end() - it >= 3) {
      const hecl::SystemChar* worldIdxStr = (*(it + 1)).c_str();
      const hecl::SystemChar* areaIdxStr = (*(it + 2)).c_str();

      hecl::SystemChar* endptr = nullptr;
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
        const hecl::SystemChar* layerStr = (*(it + 3)).c_str();
        if (!(layerStr[0] == _SYS_STR('0') && layerStr[1] == _SYS_STR('x')) &&
            (layerStr[0] == _SYS_STR('0') || layerStr[0] == _SYS_STR('1'))) {
          for (const auto* cur = layerStr; *cur != _SYS_STR('\0'); ++cur)
            if (*cur == _SYS_STR('1'))
              m_warpLayerBits |= u64(1) << (cur - layerStr);
        } else if (layerStr[0] == _SYS_STR('0') && layerStr[1] == _SYS_STR('x')) {
          m_warpMemoryRelays.emplace_back(TAreaId(hecl::StrToUl(layerStr + 2, nullptr, 16)));
        }
        ++it;
      }

      SetFlowState(EFlowState::StateSetter);
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

bool CMain::Proc() {
  // Warmup cycle overrides update
  if (m_warmupTags.size())
    return false;
  if (!m_loadedPersistentResources) {
    x128_globalObjects.m_gameResFactory->LoadPersistentResources(*g_SimplePool);
    m_loadedPersistentResources = true;
  }

  float dt = 1 / 60.f;
  if (m_cvarCommons->m_variableDt->toBoolean()) {
    auto now = delta_clock::now();
    if (m_firstFrame) {
      m_firstFrame = false;
    } else {
      using delta_duration = std::chrono::duration<float, std::ratio<1>>;
      dt = std::min(std::chrono::duration_cast<delta_duration>(now - m_prevFrameTime).count(), 1 / 30.f);
    }
    m_prevFrameTime = now;
  }

  m_console->proc();
  if (!m_console->isOpen()) {
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

  Discord_RunCallbacks();

  return x160_24_finished;
}

void CMain::Draw() {
  // Warmup cycle overrides draw
  if (m_warmupTags.size()) {
    if (m_needsWarmupClear) {
      CGraphics::g_BooMainCommandQueue->clearTarget(true, true);
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

  CGraphics::g_BooMainCommandQueue->clearTarget(true, true);
  x164_archSupport->Draw();
  m_console->draw(CGraphics::g_BooMainCommandQueue);
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
  m_console->unregisterCommand("Give");
  x128_globalObjects.m_gameResFactory->UnloadPersistentResources();
  x164_archSupport.reset();
  ShutdownSubsystems();
  CParticleSwooshShaders::Shutdown();
  CThermalColdFilter::Shutdown();
  CThermalHotFilter::Shutdown();
  CSpaceWarpFilter::Shutdown();
  CCameraBlurFilter::Shutdown();
  CXRayBlurFilter::Shutdown();
  CFogVolumePlaneShader::Shutdown();
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
  CGraphics::ShutdownBoo();
  ShutdownDiscord();
}

boo::IWindow* CMain::GetMainWindow() const { return m_mainWindow; }

#if MP1_USE_BOO

int CMain::appMain(boo::IApplication* app) {
  zeus::detectCPU();
  mainWindow = app->newWindow(_SYS_STR("Metroid Prime 1 Reimplementation vZygote"), 1);
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
  boo::GraphicsDataToken data =
      mainWindow->getMainContextDataFactory()->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool {
        renderTex = ctx.newRenderTexture(windowRect.size[0], windowRect.size[1], true, true);
        return true;
      });
  float rgba[4] = {0.2f, 0.2f, 0.2f, 1.0f};
  gfxQ->setClearColor(rgba);

  while (!xe8_b24_finished) {
    xe8_b24_finished = archSupport->Update();

    if (archSupport->isRectDirty()) {
      const boo::SWindowRect& windowRect = archSupport->getWindowRect();
      gfxQ->resizeRenderTexture(renderTex, windowRect.size[0], windowRect.size[1]);
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

} // namespace urde::MP1
