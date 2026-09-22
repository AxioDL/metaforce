#include "MetroidPrime/CMain.hpp"

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

#include "dolphin/base/PPCArch.h"
#include "dolphin/ai.h"
#include "dolphin/ar.h"
#include "dolphin/arq.h"
#include "dolphin/dvd.h"
#include "dolphin/gx.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/os/OSMemory.h"
#include "dolphin/os/OSMutex.h"
#include "dolphin/pad.h"
#include "dolphin/vi.h"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Audio/CDSPStreamManager.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Audio/CStreamAudioManager.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CARAMManager.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Input/IController.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include "Kyoto/CMemoryCardSys.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Streams/CMemoryStreamOut.hpp"
#include "Kyoto/Streams/CZipInputStream.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAudioStateWin.hpp"
#include "MetroidPrime/CConsoleOutputWindow.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CErrorOutputWindow.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/CInGameTweakManager.hpp"
#include "MetroidPrime/CMainFlow.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CMemoryCardDriver.hpp"
#include "MetroidPrime/CSplashScreen.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Factories/CCharacterFactoryBuilder.hpp"
#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CSystemState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMazeNode.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

const CFactoryFnReturn FStringTableFactory(const SObjectTag&, CInputStream&,
                                           const CVParamTransfer&);
const CFactoryFnReturn FModelFactory(const SObjectTag&, const rstl::auto_ptr< uchar >&, int,
                                     const CVParamTransfer&);
const CFactoryFnReturn FTextureFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FSkinRulesFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn AnimSourceFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FCharLayoutInfo(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FCollisionResponseDataFactory(const SObjectTag&, CInputStream&,
                                                     const CVParamTransfer&);
const CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag&, CInputStream&,
                                                  const CVParamTransfer&);
const CFactoryFnReturn FParticleFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag&, CInputStream&,
                                                    const CVParamTransfer&);
const CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag&, CInputStream&,
                                                    const CVParamTransfer&);
const CFactoryFnReturn RGuiFrameFactoryInGame(const SObjectTag&, CInputStream&,
                                              const CVParamTransfer&);
const CFactoryFnReturn FRasterFontFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FScannableObjectInfoFactory(const SObjectTag&, CInputStream&,
                                                   const CVParamTransfer&);
const CFactoryFnReturn AnimPOIDataFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FAiFiniteStateMachineFactory(const SObjectTag&, CInputStream&,
                                                    const CVParamTransfer&);
const CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag&,
                                                    const rstl::auto_ptr< uchar >&, int,
                                                    const CVParamTransfer&);
const CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag&, CInputStream&,
                                                      const CVParamTransfer&);
const CFactoryFnReturn FDecalDataFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag&, CInputStream&,
                                                     const CVParamTransfer&);
const CFactoryFnReturn FPathFindAreaFactory(const SObjectTag&, const rstl::auto_ptr< uchar >&, int,
                                            const CVParamTransfer&);
const CFactoryFnReturn FMapWorldFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FMapAreaFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FMapUniverseFactory(const SObjectTag&, CInputStream&,
                                           const CVParamTransfer&);
const CFactoryFnReturn FMidiDataFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FDependencyGroupFactory(const SObjectTag&, CInputStream&,
                                               const CVParamTransfer&);
const CFactoryFnReturn FSaveWorldFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);
const CFactoryFnReturn FHintFactory(const SObjectTag&, CInputStream&, const CVParamTransfer&);

CResFactory* gpResourceFactory;
CSimplePool* gpSimplePool;
CCubeRenderer* gpRender;
CCharacterFactoryBuilder* gpCharacterFactoryBuilder;
CGuiSys* gGuiSystem;
CStringTable* gpStringTable;
CMain* gpMain;
IController* gpController;
CGameState* gpGameState;
CMemoryCard* gpMemoryCard;
CInGameTweakManager* gpTweakManager;
const TToken< CRasterFont >* gpDefaultFont;
void* CSaveRegion::mSaveBuffer;
void* CSaveRegion::mNonVolatileSettingsBuf;
bool COsContext::mProgressiveMode;
u32 sARAMMemArray[2];
float sInfiniteLoopTime;
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
float sFramePeriod = 1.f / 50.f;
float sMaxFrameTime = 2.1f * sFramePeriod;
#define FRAME_PERIOD (sFramePeriod)
#else
#define FRAME_PERIOD (1.f / 60.f)
#endif
extern bool sIs50Hz;

#if VERSION < VERSION_GM8P_00 || VERSION >= VERSION_GM8J_00
#define GRAPHICS_FIFO_SIZE 0x60000
static uchar sGraphicsFifo[GRAPHICS_FIFO_SIZE];
#endif
ALIGNAS(CMain) static uchar sMainSpace[sizeof(CMain)];

// Generated includes
#include "MetroidPrime/DefaultFontData.inc"
#include "MetroidPrime/DefaultFontTexture.inc"

struct SAudioGroupInfo {
  const char* name;
  uchar groupId;
};

static const SAudioGroupInfo skPreLoadGroups[] = {
    {"Misc_AGSC", 39},    {"MiscSamus_AGSC", 41}, {"UI_AGSC", 40},
    {"Weapons_AGSC", 43}, {"ZZZ_AGSC", 65},
};
// sdata
bool sProgressiveModePrompt = true;

extern "C" void OSGetSavedRegion(void** start, void** end);
extern "C" void OSSetSaveRegion(void* start, void* end);
static inline void AddFrameTime(TReservedAverage< float, 4 >& average, const float& value) {
  if (average.size() < average.capacity()) {
    average.push_back(value);
  }
  for (int i = average.size() - 1; i > 0; --i) {
    average[i] = average[i - 1];
  }
  average[0] = value;
}

#define UNUSED_STACK_VAL 0x7337D00D

CSaveRegion::CSaveRegion(CMain& main) {
  void* end;
  OSGetSavedRegion(&mNonVolatileSettingsBuf, &end);
  OSSetSaveRegion(nullptr, nullptr);
  mSaveBuffer = main.OsContext().AllocFromArena(128);
}

int main(int argc, char** argv) {
  DVDSetAutoFatalMessaging(TRUE);
  SetErrorHandlers();
  CMain* main = new (&sMainSpace) CMain();
  gpMain->RsMain(argc, argv);
  main->~CMain();
  return 0;
}

extern "C" void* __sys_alloc(const size_t len) { return CMemory::Alloc(len); }

extern "C" void __sys_free(const void* ptr) { CMemory::Free(ptr); }

COsContext& CMain::OpenWindow() {
  if (CSaveRegion::GetNonVolatileSettingsBuffer() != nullptr) {
    CMemoryInStream stream(CSaveRegion::GetNonVolatileSettingsBuffer(), 128);
    COsContext::SetProgressiveMode(stream.ReadBits(1));
  }
#if VERSION < VERSION_GM8P_00 || VERSION >= VERSION_GM8J_00
  x0_osContext.OpenWindow("Metaforce", 0, 0, 640, 480, true);
#endif
  return x0_osContext;
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
int CMain::GetLanguage() {
  int language = x0_osContext.GetLanguage();
  if (language == 5) {
    language = 0;
  }
  return language;
}
#endif

#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
void CMain::SetTiming() { sFramePeriod = sIs50Hz ? 1.f / 50.f : 1.f / 60.f; }
#endif

CMain::CMain()
: x0_osContext(true, true)
, x6c_saveRegion(*this)
, x6d_memorySys(OpenWindow(), CMemorySys::GetGameAllocator())
, xe8_unknown(0.0)
, x118_averageTickTime(0.f)
, x11c_averageDrawTime(0.f)
, x120_softResetHoldTime(0.f)
, x124_resetInputDelay(0.f)
, x128_gameGlobalObjects(nullptr)
, x12c_restartMode(kRM_Default)
, x130_frameTimes(0xF4240)
, x15c_frameTimeIdx(0)
, x160_24_finished(false)
, x160_25_mfGameBuilt(false)
, x160_26_screenFading(false)
, x160_27_resetButtonHeld(false)
, x160_28_manageCard(false)
, x160_29_resetRequested(false)
, x160_30_gameExitReset(false)
, x160_31_cardBusy(false)
, x161_24_gameFrameDrawn(false)
, x164_archSupport(nullptr) {
  gpMain = this;
}

CMain::~CMain() {}

void CMain::InitializeSubsystems() {
  ARInit(sARAMMemArray, 2);
  ARAlloc(0x5fc000);
  CARAMManager::PreInitializeAlloc(0x5fc000);
  ARQInit();
  OSThread* thread = OSGetCurrentThread();
  printf("Protecting stack...  ");
  uchar* stackEnd =
      reinterpret_cast< uchar* >(ALIGN_UP(reinterpret_cast< uintptr_t >(thread->stackEnd), 0x400));
  uchar* stackBase = thread->stackBase;
  OSProtectRange(OS_PROTECT_CHAN3, stackEnd, 0x400, OS_PROTECT_CONTROL_NONE);

  uchar* ptr = stackEnd + 0x400;
  for (; ptr < stackBase - 0x2000; ptr += 4) {
    *reinterpret_cast< int* >(ptr) = UNUSED_STACK_VAL;
  }

  DCFlushRange(stackEnd + 0x400, static_cast< uint >(stackBase - 0x2000 - (stackEnd + 0x400)));
  printf("Stack: 0x%8.8x down to 0x%8.8x\n", thread->stackBase, thread->stackEnd);
  CElementGen::Initialize();
  CAnimData::InitializeCache();
  CARAMManager::Initialize(0x800);
  CDecalManager::Initialize();
  CFrameDelayedKiller::Initialize();
}

void CMain::ShutdownSubsystems() {
  CFrameDelayedKiller::ShutDown();
  CDecalManager::ShutDown();
  CElementGen::ShutDown();
  CAnimData::FreeCache();

  OSThread* thread = OSGetCurrentThread();
  uchar* stackEnd =
      reinterpret_cast< uchar* >(ALIGN_UP(reinterpret_cast< uintptr_t >(thread->stackEnd), 0x400));
  uchar* stackBase = thread->stackBase;

  uchar* ptr = stackEnd + 0x400;
  for (; ptr < stackBase - 0x2000; ptr += 4) {
    if (*reinterpret_cast< uint* >(ptr) != UNUSED_STACK_VAL) {
      break;
    }
  }
  const int used = static_cast< int >(stackBase - 0x2000 - ptr) + 0x2000;
  OSReport("Stack usage: %d bytes (%dk)\n", used, static_cast< uint >(used) / 1024);
}

CGameGlobalObjects::CGameGlobalObjects(COsContext& osContext, CMemorySys& memorySys)
: xcc_simplePool(x4_resFactory)
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
, x130_graphicsSys(osContext, memorySys, COsContext::GetProgressiveMode())
#else
, x130_graphicsSys(osContext, memorySys, GRAPHICS_FIFO_SIZE, sGraphicsFifo)
#endif
, x134_gameState(rs_new CGameState())
, x150_inGameTweakManager(rs_new CInGameTweakManager())
, x154_defaultFont(LoadDefaultFont()) {
  gpResourceFactory = &x4_resFactory;
  gpSimplePool = &xcc_simplePool;
  gpCharacterFactoryBuilder = &xec_characterFactoryBuilder;
  gpGameState = x134_gameState.get();
  gpTweakManager = x150_inGameTweakManager.get();
  gpDefaultFont = &x154_defaultFont;
}

CRasterFont* CGameGlobalObjects::LoadDefaultFont() {
  CZipInputStream fontDataStream(
      rs_new CMemoryInStream(sDefaultFontData, sizeof(sDefaultFontData)));
  CRasterFont* font = rs_new CRasterFont(fontDataStream, nullptr);
  CZipInputStream fontTextureStream(
      rs_new CMemoryInStream(sDefaultFontTexture, sizeof(sDefaultFontTexture)));
  font->SetTexture(rs_new CTexture(fontTextureStream, CTexture::kAM_Zero, CTexture::kBK_Zero));
  return font;
}

void CGameGlobalObjects::PostInitialize(COsContext& osContext, CMemorySys& memorySys) {
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
  CMain::SetTiming();
#endif
#if VERSION >= VERSION_GM8E_01
  AddPaksAndFactories(osContext);
#else
  AddPaksAndFactories();
#endif
  LoadStringTable();
  printf("Initializing renderer...\n");
  x14c_renderer = Renderer::AllocateRenderer(xcc_simplePool, osContext, memorySys, x4_resFactory);
  gpRender = reinterpret_cast< CCubeRenderer* >(x14c_renderer.get());
  CEnvFxManager::Initialize();
  CScriptMazeNode::LoadMazeSeeds();
}

void CGameGlobalObjects::LoadStringTable() {
  x13c_stringTable = gpSimplePool->GetObj("STRG_Main");
  gpStringTable = **x13c_stringTable;
}

void InitializeApplicationUI(CGuiSys&);

void InfiniteLoopAlarm(OSAlarm* alarm, OSContext* context) {
  if (sInfiniteLoopTime >= 10.f) {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    OSCancelAlarm(alarm);
    rs_debugger_printf("SKIP4INFINITE LOOP");
#else
    rs_debugger_printf("INFINITE LOOP");
#endif
  }
  sInfiniteLoopTime += alarm->period / OS_TIMER_CLOCK;
}

CGameArchitectureSupport::CGameArchitectureSupport(COsContext& osContext)
: x0_audioSys(0x30, 0x30, 0x30, 0x30, 0x5fc000)
, x30_inputGenerator(&osContext, gpTweakPlayer->GetLeftAnalogMax(),
                     gpTweakPlayer->GetRightAnalogMax())
, x44_guiSys(gpResourceFactory, gpSimplePool, CGuiSys::kUM_Zero)
, x78_gameFrameCount(0)
, x7c_tickRemainder(0.f)
, x80_previousTickRemainder2(0.f)
, x84_previousTickRemainder(0.f)
, x88_audioLoadStatus(kALS_Uninitialized)
, xc8_infiniteLoopAlarmSet(false) {
  CAudioSys::SysSetVolume(0x7F, 0, 0xFF);
  CAudioSys::SetDefaultVolumeScale(0x75);
  CAudioSys::SetVolumeScale(CAudioSys::GetDefaultVolumeScale());
  CDSPStreamManager::Initialize();
  CStreamAudioManager::SetMusicVolume(0x7F);
  CAudioSys::TrkSetSampleRate(kTSR_One);
  gpMain->SetMaxSpeed(false);
  gpMain->ResetGameState();
  CIOWinManager& ioWinManager = x58_ioWinMgr;
  if (!gpTweakGame->GetSplashScreensDisabled()) {
    ioWinManager.AddIOWin(rs_new CSplashScreen(CSplashScreen::kSplashScreen_Nintendo), 1000, 10000);
  }
  ioWinManager.AddIOWin(rs_new CMainFlow(), 0, 0);
  ioWinManager.AddIOWin(rs_new CConsoleOutputWindow(8, 5.f, 0.75f), 100, 0);
  ioWinManager.AddIOWin(rs_new CAudioStateWin(), 100, -1);
  ioWinManager.AddIOWin(rs_new CErrorOutputWindow(CErrorOutputWindow::kF_Zero), 10000, 100000);
#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02
  InitializeApplicationUI(x44_guiSys);
#endif
  CGuiSys::SetGlobalGuiSys(&x44_guiSys);
  gpController = x30_inputGenerator.GetController();
  gpGameState->GameOptions().EnsureOptions();
  sInfiniteLoopTime = 0.f;
  OSSetPeriodicAlarm(&xa0_infiniteLoopAlarm, OSGetTime(), (float)OS_TIMER_CLOCK, InfiniteLoopAlarm);
  xc8_infiniteLoopAlarmSet = true;
}

CGameArchitectureSupport::~CGameArchitectureSupport() {
  if (xc8_infiniteLoopAlarmSet) {
    OSCancelAlarm(&xa0_infiniteLoopAlarm);
    xc8_infiniteLoopAlarmSet = false;
  }
  x58_ioWinMgr.RemoveAllIOWins();
  UnloadAudio();
  CSfxManager::Shutdown();
  CDSPStreamManager::Shutdown();
}

bool CGameArchitectureSupport::UpdateTicks() {
  bool terminate = false;
  const BOOL interrupts = OSDisableInterrupts();
  const float elapsed = x20_tickStopwatch.GetElapsedTime();
  x20_tickStopwatch.Reset();
  OSRestoreInterrupts(interrupts);
  sInfiniteLoopTime = 0.f;
  x7c_tickRemainder += elapsed;
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
  if (gpMain->GetScreenFading() || elapsed > sMaxFrameTime) {
#else
  if (gpMain->GetScreenFading() || elapsed > 0.035f) {
#endif
    x7c_tickRemainder = FRAME_PERIOD;
  }

  bool first = true;
  x4_archQueue.Push(MakeMsg::CreateFrameBegin(kAMT_Game, x78_gameFrameCount));
  while (first || x7c_tickRemainder >= FRAME_PERIOD) {
    first = false;
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
    const float tickPeriod = FRAME_PERIOD;
#else
    static const float tickPeriod = FRAME_PERIOD;
#endif
    if (!x30_inputGenerator.Update(FRAME_PERIOD, x4_archQueue)) {
      terminate = true;
    }
    x4_archQueue.Push(MakeMsg::CreateTimerTick(kAMT_Game, tickPeriod));
    x7c_tickRemainder -= FRAME_PERIOD;
    x58_ioWinMgr.PumpMessages(x4_archQueue);
  }

  if (close_enough((x80_previousTickRemainder2 - x84_previousTickRemainder) +
                       (x84_previousTickRemainder - x7c_tickRemainder),
                   0.f, 0.00005f)) {
    x7c_tickRemainder = 0.f;
  }
  x80_previousTickRemainder2 = x84_previousTickRemainder;
  x84_previousTickRemainder = x7c_tickRemainder;
  x58_ioWinMgr.PumpMessages(x4_archQueue);
  return !terminate;
}

void CGameArchitectureSupport::Update() {
  gpGameState->WorldTransitionManager()->TouchModels();
  x4_archQueue.Push(MakeMsg::CreateFrameEnd(kAMT_Game, x78_gameFrameCount));
  x58_ioWinMgr.PumpMessages(x4_archQueue);
}

void CGameArchitectureSupport::PreloadAudio() {
  if (x88_audioLoadStatus == kALS_Uninitialized) {
    x8c_pendingAudioGroups = rstl::vector< CToken >();
    x8c_pendingAudioGroups.reserve(5);
    for (int i = 0; i < 5; ++i) {
      CToken group = gpSimplePool->GetObj(skPreLoadGroups[i].name);
      if (i == 0) {
        group.Lock();
      }
      x8c_pendingAudioGroups.push_back(group);
    }
    x88_audioLoadStatus = kALS_Loading;
  }
}

bool CGameArchitectureSupport::LoadAudio() {
  if (x88_audioLoadStatus == kALS_Loaded) {
    return true;
  }

  bool loaded = true;
  for (int i = 0; i < sizeof(skPreLoadGroups) / sizeof(skPreLoadGroups[0]); ++i) {
    CToken& token = *(x8c_pendingAudioGroups.begin() + i);
    const SAudioGroupInfo& info = skPreLoadGroups[i];
    if (token.IsLocked()) {
      if (token.IsLoaded()) {
        TToken< CAudioGrpSetLoc > group(token);
        if (!CAudioSys::SysIsGroupSetLoaded(group->GetGroupSetName())) {
          const CAssetId id = gpResourceFactory->GetResourceIdByName(info.name)->GetId();
          CAudioSys::SysLoadGroupSet(group, group->GetGroupSetName(), id);
          const rstl::string& name = group->GetGroupSetName();
          CAudioSys::SysPushGroupIntoARAM(name, info.groupId);
          CAudioSys::SysUnloadSampleData(name);
        }
      } else {
        loaded = false;
        break;
      }
    } else {
      loaded = false;
      token.Lock();
      break;
    }
  }
  if (!loaded) {
    return false;
  }
  CSfxManager::LoadTranslationTable(gpSimplePool,
                                    gpResourceFactory->GetResourceIdByName("sound_lookup"));
  x8c_pendingAudioGroups = rstl::vector< CToken >();
  x88_audioLoadStatus = kALS_Loaded;
  return true;
}

bool CMain::LoadAudio() {
  if (x164_archSupport != nullptr) {
    return x164_archSupport->LoadAudio();
  }
  return true;
}

void CGameArchitectureSupport::UnloadAudio() {
  if (x88_audioLoadStatus == kALS_Loaded) {
    for (uint i = 0; i < 5; ++i) {
      CAudioSys::SysPopGroupFromARAM();
      rstl::string name = CAudioSys::SysGetGroupSetName(
          gpResourceFactory->GetResourceIdByName(skPreLoadGroups[4 - i].name)->GetId());
      CAudioSys::SysUnloadGroupSet(name);
    }
  }
  x8c_pendingAudioGroups = rstl::vector< CToken >();
  x88_audioLoadStatus = kALS_Uninitialized;
}

void CMain::MemoryCardInitializePump() {
  if (gpMemoryCard == nullptr) {
    if (x128_gameGlobalObjects->MemoryCard().get() == nullptr) {
      x128_gameGlobalObjects->MemoryCard() = rs_new CMemoryCard();
    }
    CMemoryCard* card = x128_gameGlobalObjects->MemoryCard().get();
    if (card->InitializePump()) {
      gpMemoryCard = card;
      gpGameState->InitializeMemoryStates();
    }
  }
}

#if VERSION >= VERSION_GM8E_01
void CGameGlobalObjects::AddPaksAndFactories(const COsContext& osContext) {
#else
void CGameGlobalObjects::AddPaksAndFactories() {
#endif
  CResFactory& factory = *gpResourceFactory;
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  CGraphics::SetModelMatrix(CTransform4f::Identity());
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  if (CDvdFile::FileExists("Strings.pak")) {
    factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:Strings"), false, false);
  }
#endif
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:Tweaks"), false, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("NoARAM"), false, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("AudioGrp"), false, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:MiscData"), false, false);

  CErrorOutputWindow errorWindow(CErrorOutputWindow::kF_One);
  CGraphics::SetIsBeginSceneClearFb(true);
  CGraphics::SetViewport(0, 0, CGraphics::GetViewportWidth(), CGraphics::GetViewportHeight());
#if VERSION >= VERSION_GM8E_01
  rstl::single_ptr< IController > controller(IController::Create(osContext));
  gpController = controller.get();
#endif
  while (!factory.GetResLoader().AreAllPaksLoaded()) {
    gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
    errorWindow.Update();
    CGraphics::BeginScene();
    errorWindow.ShowMessage();
    CGraphics::EndScene();
#if VERSION >= VERSION_GM8E_01
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    if (controller.get() != nullptr) {
      controller->Poll();
    }
#else
    controller->Poll();
#endif
    gpMain->CheckReset();
#endif
  }
#if VERSION >= VERSION_GM8E_01
  gpController = nullptr;
#endif

  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:SamusGun"), true, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:TestAnim"), true, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:SamGunFx"), true, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:MidiData"), false, false);
  factory.GetResLoader().AddPakFileAsync(rstl::string_l("aram:GGuiSys"), false, false);

  factory.GetFactoryMgr().AddFactory('STRG', FStringTableFactory);
  factory.GetFactoryMgr().AddFactory('CMDL', FModelFactory);
  factory.GetFactoryMgr().AddFactory('TXTR', FTextureFactory);
  factory.GetFactoryMgr().AddFactory('CSKR', FSkinRulesFactory);
  factory.GetFactoryMgr().AddFactory('ANIM', AnimSourceFactory);
  factory.GetFactoryMgr().AddFactory('CINF', FCharLayoutInfo);
  factory.GetFactoryMgr().AddFactory('ANCS', FAnimCharacterSet);
  factory.GetFactoryMgr().AddFactory('CRSC', FCollisionResponseDataFactory);
  factory.GetFactoryMgr().AddFactory('SWHC', FParticleSwooshDataFactory);
  factory.GetFactoryMgr().AddFactory('PART', FParticleFactory);
  factory.GetFactoryMgr().AddFactory('ELSC', FParticleElectricDataFactory);
  factory.GetFactoryMgr().AddFactory('WPSC', FProjectileWeaponDataFactory);
  factory.GetFactoryMgr().AddFactory('FRME', RGuiFrameFactoryInGame);
  factory.GetFactoryMgr().AddFactory('FONT', FRasterFontFactory);
  factory.GetFactoryMgr().AddFactory('SCAN', FScannableObjectInfoFactory);
  factory.GetFactoryMgr().AddFactory('EVNT', AnimPOIDataFactory);
  factory.GetFactoryMgr().AddFactory('AFSM', FAiFiniteStateMachineFactory);
  factory.GetFactoryMgr().AddFactory('AGSC', FAudioGroupSetLocDataFactory);
  factory.GetFactoryMgr().AddFactory('DCLN', FCollidableOBBTreeGroupFactory);
  factory.GetFactoryMgr().AddFactory('DPSC', FDecalDataFactory);
  factory.GetFactoryMgr().AddFactory('ATBL', FAudioTranslationTableFactory);
  factory.GetFactoryMgr().AddFactory('PATH', FPathFindAreaFactory);
  factory.GetFactoryMgr().AddFactory('MAPW', FMapWorldFactory);
  factory.GetFactoryMgr().AddFactory('MAPA', FMapAreaFactory);
  factory.GetFactoryMgr().AddFactory('MAPU', FMapUniverseFactory);
  factory.GetFactoryMgr().AddFactory('CSNG', FMidiDataFactory);
  factory.GetFactoryMgr().AddFactory('DGRP', FDependencyGroupFactory);
  factory.GetFactoryMgr().AddFactory('SAVW', FSaveWorldFactory);
  factory.GetFactoryMgr().AddFactory('HINT', FHintFactory);
}

void CMain::FillInAssetIDs() {
  const SObjectTag* tag =
      gpResourceFactory->GetResourceIdByName(gpTweakGame->GetDefaultRoom().data());
  if (tag != nullptr) {
    gpGameState->SetCurrentWorldId(tag->GetId());
  }
}

void CMain::DoPredrawMetrics() {}

void CMain::DrawDebugMetrics(double, CStopwatch&) {}

bool CMain::CheckTerminate() { return false; }

bool CMain::CheckReset() {
  const BOOL resetButton = OSGetResetButtonState();
  const CControllerGamepadData& pad = gpController->GetGamepadData(0);
  if (pad.GetButton(kBU_B).GetIsPressed() && pad.GetButton(kBU_X).GetIsPressed() &&
      pad.GetButton(kBU_Start).GetIsPressed()) {
    if (x124_resetInputDelay >= 0.5f) {
      x120_softResetHoldTime += FRAME_PERIOD;
      if (x120_softResetHoldTime > 0.5f) {
        x160_27_resetButtonHeld = true;
      }
    }
  } else {
    if (x124_resetInputDelay < 0.5f) {
      x124_resetInputDelay += FRAME_PERIOD;
    }
    x120_softResetHoldTime = 0.f;
  }
  if (!resetButton && x160_27_resetButtonHeld) {
    x160_29_resetRequested = true;
  }

  if (!x160_31_cardBusy &&
      (x160_29_resetRequested || x160_28_manageCard || x160_30_gameExitReset)) {
    if (x164_archSupport != nullptr && x164_archSupport->IsInfiniteLoopAlarmSet()) {
      OSCancelAlarm(&x164_archSupport->GetInfiniteLoopAlarm());
      x164_archSupport->SetInfiniteLoopAlarmSet(false);
    }
    GXDrawDone();
    GXAbortFrame();
#if VERSION < VERSION_GM8E_01
    CAudioSys::TrkFlushTracks();
    AISetStreamPlayState(0);
#endif
    if (!x160_30_gameExitReset) {
      gpGameState->GameOptions() = CGameOptions();
    } else {
      CGameOptions& options = gpGameState->GameOptions();
      options.SetScreenBrightness(4, false);
      options.SetScreenPositionX(0, false);
      options.SetScreenPositionY(0, false);
      options.SetScreenStretch(0, false);
    }
    {
      CMemoryStreamOut stream(CSaveRegion::GetSaveBuffer(), 128);
      stream.WriteBits(CGraphics::GetProgressiveMode() ? 1 : 0, 1);
      gpGameState->GameOptions().PutTo(stream);
      stream.WriteBits(sProgressiveModePrompt ? 1 : 0, 1);
      stream.Flush();
    }
    gpGameState->GameOptions().EnsureOptions();
    DCFlushRange(CSaveRegion::GetSaveBuffer(), 128);
    OSSetSaveRegion(CSaveRegion::GetSaveBuffer(),
                    static_cast< uchar* >(CSaveRegion::GetSaveBuffer()) + 128);
    VISetBlack(TRUE);
    VIFlush();
    VIWaitForRetrace();
    if (x160_28_manageCard) {
      OSResetSystem(OS_RESET_HOTRESET, 0, TRUE);
    } else if (DVDCheckDisk()) {
#if VERSION < VERSION_GM8P_00 || VERSION >= VERSION_GM8J_00
      DVDCancelAll();
      DVDCommandBlock block;
      DVDCancelStream(&block);
#endif
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
      AISetStreamPlayState(0);
      if (CAudioSys::mInitialized) {
        sndQuit();
      }
#elif VERSION >= VERSION_GM8E_01
      if (CAudioSys::mInitialized) {
        CAudioSys::TrkFlushTracks();
      }
      AISetStreamPlayState(0);
      if (CAudioSys::mInitialized) {
        sndQuit();
      }
#endif
      OSResetSystem(OS_RESET_RESTART, 0, FALSE);
    } else {
      OSResetSystem(OS_RESET_HOTRESET, 0, FALSE);
    }
    x160_27_resetButtonHeld = false;
    x160_29_resetRequested = false;
    x160_30_gameExitReset = false;
    x160_28_manageCard = false;
    return true;
  }
  x160_27_resetButtonHeld = resetButton;
  return false;
}

int CMain::RsMain(int argc, const char* const* argv) {
  PPCSetFpIEEEMode();
  CStopwatch timer;
  LCEnable();

  rstl::single_ptr< CGameGlobalObjects > gameGlobalObjects(
      rs_new CGameGlobalObjects(x0_osContext, x6d_memorySys));
  x128_gameGlobalObjects = gameGlobalObjects.get();
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CMemoryCardDriver::LoadLanguageFromCard(0);
  CStringTable::SetLanguage(GetLanguage());
#endif

  for (int i = 0; i < 4; ++i) {
    AddFrameTime(xf0_tickTimes, 0.3f);
    AddFrameTime(x104_drawTimes, 0.2f);
  }

  x118_averageTickTime = 0.3f;
  x11c_averageDrawTime = 0.2f;
  InitializeSubsystems();
  gameGlobalObjects->PostInitialize(x0_osContext, x6d_memorySys);
  x70_tweaks.RegisterTweaks();
  AddWorldPaks();

  {
    rstl::string str;
    bool logAudioTweaks;
    if (gpTweakManager->ReadFromMemoryCard(rstl::string_l("AudioTweaks"))) {
      str = rstl::string_l("Loaded audio tweaks from memory card\n");
      logAudioTweaks = true;
    } else {
      str = rstl::string_l("FAILED to load audio tweaks from memory card\n");
      logAudioTweaks = true;
    }

    FillInAssetIDs();

    rstl::single_ptr< CGameArchitectureSupport > archSupport(
        rs_new CGameArchitectureSupport(x0_osContext));
    x164_archSupport = archSupport.get();
    archSupport->PreloadAudio();

    srand(timer.GetElapsedMicros());

    if (CSaveRegion::GetNonVolatileSettingsBuffer() != nullptr) {
      CMemoryInStream stream(CSaveRegion::GetNonVolatileSettingsBuffer(), 0x80);
      stream.ReadBits(1);
      gpGameState->GameOptions() = CGameOptions(stream);
      gpGameState->GameOptions().EnsureOptions();
      sProgressiveModePrompt = stream.ReadBits(1);
    }

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    CDvdFile::FileExists("Strings.pak");
#endif
    while (!x160_24_finished) {
#if VERSION >= VERSION_GM8P_00 && VERSION < VERSION_GM8J_00
      SetTiming();
#endif
      archSupport->GetStopwatch2().Reset();
      gpResourceFactory->GetResLoader().AsyncIdlePakLoading();
      if (gpMemoryCard == nullptr && gpResourceFactory->GetResLoader().AreAllPaksLoaded()) {
        MemoryCardInitializePump();
      }
      CARAMManager::CollectGarbage();
      CARAMToken::UpdateAllDMAs();
      if (!archSupport->UpdateTicks()) {
        x160_24_finished = true;
      }
      double t1 = archSupport->GetStopwatch2().GetElapsedTime();
      AddFrameTime(xf0_tickTimes, t1 / FRAME_PERIOD);
      x118_averageTickTime = xf0_tickTimes.GetAverage().data();
      archSupport->GetStopwatch2().Reset();
      DoPredrawMetrics();

      if (logAudioTweaks) {
        logAudioTweaks = false;
        // rs_log_print(str.data());
      }
      if (!x160_26_screenFading) {
        gpRender->BeginScene();
        archSupport->GetIOWinManager().Draw();
        DrawDebugMetrics(t1, archSupport->GetStopwatch2());

        double t2 = archSupport->GetStopwatch2().GetElapsedTime();
        AddFrameTime(x104_drawTimes, t2 / FRAME_PERIOD);
        x11c_averageDrawTime = x104_drawTimes.GetAverage().data();

        uint idleMicros;
        double idleTime = (FRAME_PERIOD - (t1 + t2)) - 0.00075;
        if (idleTime > 0)
          idleMicros = idleTime * 1000000;
        else
          idleMicros = 0;
        AsyncIdle(idleMicros);

        gpRender->EndScene();

        if (x161_24_gameFrameDrawn) {
          ++archSupport->GetFramesDrawn();
          x161_24_gameFrameDrawn = false;
        }
      } else {
        gpResourceFactory->AsyncIdle(1000000);
      }

      archSupport->Update();
      CSfxManager::Update(FRAME_PERIOD);
      UpdateStreamedAudio();

      if (CheckTerminate())
        break;
      bool needsReset = false;
      if (archSupport->GetIOWinManager().IsEmpty()) {
        // rs_log_print("IOWinManager got empty. Resetting game architecture\n");
        needsReset = true;
      } else if (CheckReset()) {
        // rs_log_print("Reset pressed...\n");
        needsReset = true;
      }
      if (needsReset) {
        x12c_restartMode = kRM_Default;
        CStreamAudioManager::StopAll();
        PADRecalibrate(0xf0000000);
        CGraphics::SetIsBeginSceneClearFb(true);
        CGraphics::BeginScene();
        CGraphics::EndScene();
        CFrameDelayedKiller::StallAndFlushAllAllocations();

        archSupport = nullptr;
        CGameArchitectureSupport* tmp = rs_new CGameArchitectureSupport(x0_osContext);
        archSupport = tmp;
        x164_archSupport = archSupport.get();
        tmp->PreloadAudio();
      }
      CheckTweakManagerDebugOptions();
    }
  }
  ShutdownSubsystems();
  gameGlobalObjects = nullptr;
  CARAMManager::Shutdown();
  return 0;
}

void CMain::AsyncIdle(uint time) {
  if (time < 500) {
    uint total = 0;
    for (int i = 0; i < x130_frameTimes.capacity(); ++i) {
      total += x130_frameTimes[i];
    }
    if (total < 500 * x130_frameTimes.capacity()) {
      time = 500;
    } else {
      time = 0;
    }
  }
  if (time != 0) {
    gpResourceFactory->AsyncIdle(time);
  }
  x130_frameTimes[x15c_frameTimeIdx] = time;
  x15c_frameTimeIdx = x15c_frameTimeIdx + 1;
  if (x15c_frameTimeIdx >= x130_frameTimes.capacity()) {
    x15c_frameTimeIdx = 0;
  }
}

namespace rstl {
string string_l(const char* data) { return string(string::literal_t(), data); }

string operator+(const string& a, const string& b) {
  string result(a);
  result.append(b);
  return result;
}
} // namespace rstl

void CMain::AddWorldPaks() {
  rstl::rmemory_allocator allocator;
  rstl::string basePath = gpTweakGame->GetWorldPrefix();
  for (int i = 0; i < 9; ++i) {
    rstl::string pak =
        basePath +
        (i == 0 ? rstl::string_l("") : rstl::string(CBasics::Stringize("%d", i), -1, allocator));
    if (CDvdFile::FileExists((pak + rstl::string_l(".pak")).data())) {
      gpResourceFactory->GetResLoader().AddPakFileAsync(pak, false, true);
    }
  }
}

void CMain::EnsureWorldPakReady(CAssetId id) {
  CResLoader& resLoader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < resLoader.GetPakCount(); ++i) {
    bool notInNameList = true;
    CPakFile* pakFile = resLoader.GetPakFile(i);
    if (pakFile->IsWorldPak()) {
      rstl::vector< rstl::pair< rstl::string, SObjectTag > > nameList = pakFile->NameList();
      rstl::vector< rstl::pair< rstl::string, SObjectTag > >::iterator cur = nameList.begin();
      while (cur != nameList.end()) {
        if (cur->second.GetId() == id) {
          notInNameList = false;
        }
        ++cur;
      }
      if (notInNameList) {
        pakFile->sub_8036742c();
      } else {
        pakFile->EnsureWorldPakReady();
      }
    }
  }
}

void CMain::EnsureWorldPaksReady(void) {
  CResLoader& resLoader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < resLoader.GetPakCount(); ++i) {
    CPakFile* file = resLoader.GetPakFile(i);
    if (file->IsWorldPak()) {
      file->EnsureWorldPakReady();
    }
  }
}

void CMain::CheckTweakManagerDebugOptions() {}

void CMain::RefreshGameState() {
  CSystemState systemState = gpGameState->SystemState();
  uint saveIdx = gpGameState->SaveIdx();
  u64 cardSerial = gpGameState->CardSerial();
  rstl::vector< uchar > backupBuf = gpGameState->BackupBuf();
  CGameOptions gameOptions = gpGameState->GameOptions();
  x128_gameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  {
    CMemoryInStream stream(backupBuf.data(), backupBuf.size(), CMemoryInStream::kOS_NotOwned);
    x128_gameGlobalObjects->GameState() = rs_new CGameState(stream, saveIdx);
  }
  gpGameState = x128_gameGlobalObjects->GameState().get();
  gpGameState->SystemState() = systemState;
  gpGameState->GameOptions() = gameOptions;
  gpGameState->GameOptions().EnsureOptions();
  gpGameState->CardSerial() = cardSerial;
  gpGameState->PlayerState()->SetIsFusionEnabled(gpGameState->SystemState().GetHasFusion());
}

void CMain::StreamNewGameState(CInputStream& in, int saveIdx) {
  bool hasFusion = gpGameState->SystemState().GetHasFusion();
  x128_gameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  x128_gameGlobalObjects->GameState() = rs_new CGameState(in, saveIdx);
  gpGameState = x128_gameGlobalObjects->GameState().get();
  gpGameState->SystemState().SetHasFusion(hasFusion);
  gpGameState->PlayerState()->SetIsFusionEnabled(gpGameState->SystemState().GetHasFusion());
  gpGameState->HintOptions().SetHintNextTime();
}

void CMain::ResetGameState() {
  CSystemState systemState = gpGameState->SystemState();
  CGameOptions gameOptions = gpGameState->GameOptions();
  x128_gameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  x128_gameGlobalObjects->GameState() = rs_new CGameState();
  gpGameState = x128_gameGlobalObjects->GameState().get();
  gpGameState->SystemState() = systemState;
  gpGameState->GameOptions() = gameOptions;
  gpGameState->GameOptions().EnsureOptions();
  gpGameState->PlayerState()->SetIsFusionEnabled(gpGameState->SystemState().GetHasFusion());
}

void CMain::RegisterResourceTweaks() { x70_tweaks.RegisterResourceTweaks(); }

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void CMain::ReloadStringTables() {
  rstl::vector< SObjectTag > tags = gpSimplePool->GetReferencedTags();
  for (rstl::vector< SObjectTag >::const_iterator it = tags.begin(); it != tags.end(); ++it) {
    if (it->GetType() == 'STRG' && gpSimplePool->GetObj(*it).IsLoaded()) {
      TLockedToken< CStringTable > table = gpSimplePool->GetObj(*it);
      table->Reload(it->GetId(), *gpResourceFactory);
    }
  }
}

#endif

void CMain::UpdateStreamedAudio() { CStreamAudioManager::Update(FRAME_PERIOD); }
