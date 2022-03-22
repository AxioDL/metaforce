#pragma once

#include "Runtime/IMain.hpp"
#include "Runtime/MP1/CTweaks.hpp"
#include "Runtime/MP1/CPlayMovie.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/CBasics.hpp"
#include "Runtime/CMemoryCardSys.hpp"
#include "Runtime/CResFactory.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/Character/CAssetFactory.hpp"
#include "Runtime/World/CAi.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/ImGuiConsole.hpp"
#include "Runtime/MP1/CInGameTweakManager.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Character/CAnimData.hpp"
#include "Runtime/Particle/CDecalManager.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Audio/CAudioSys.hpp"
#include "Runtime/Input/CInputGenerator.hpp"
#include "Runtime/GuiSys/CGuiSys.hpp"
#include "Runtime/CIOWinManager.hpp"
#include "Runtime/GuiSys/CSplashScreen.hpp"
#include "Runtime/MP1/CMainFlow.hpp"
#include "Runtime/GuiSys/CConsoleOutputWindow.hpp"
#include "Runtime/GuiSys/CErrorOutputWindow.hpp"
#include "Runtime/GuiSys/CTextParser.hpp"
#include "Runtime/MP1/CAudioStateWin.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/CArchitectureQueue.hpp"
#include "Runtime/CTimeProvider.hpp"
#include "Runtime/GuiSys/CTextExecuteBuffer.hpp"
#include "Runtime/MP1/Tweaks/CTweakPlayer.hpp"
#include "Runtime/MP1/Tweaks/CTweakGame.hpp"
#include "Runtime/ConsoleVariables/CVarCommons.hpp"

struct DiscordUser;

namespace metaforce {
class IFactory;
class IObjectStore;

namespace MP1 {
class CMain;

class CGameGlobalObjects {
  friend class CMain;

  std::unique_ptr<CSimplePool> m_gameSimplePool;
  std::unique_ptr<CResFactory> m_gameResFactory;
  std::unique_ptr<CMemoryCardSys> x0_memoryCardSys;
  IFactory* x4_resFactory;
  CSimplePool* xcc_simplePool;
  CCharacterFactoryBuilder xec_charFactoryBuilder;
  CAiFuncMap x110_aiFuncMap;
  std::unique_ptr<CGameState> x134_gameState;
  TLockedToken<CStringTable> x13c_mainStringTable;
  CInGameTweakManager x150_tweakManager;
  std::unique_ptr<IRenderer> m_renderer;
  TLockedToken<CTextureCache> m_textureCache;

  void LoadStringTable() {
    x13c_mainStringTable = g_SimplePool->GetObj("STRG_Main");
    g_MainStringTable = x13c_mainStringTable.GetObj();
  }
  void LoadTextureCache() {
    m_textureCache = g_SimplePool->GetObj("TextureCache"sv);
    g_TextureCache = m_textureCache.GetObj();
  }
  void AddPaksAndFactories();
  static IRenderer* AllocateRenderer(IObjectStore& store, IFactory& resFactory) {
    g_Renderer = new CCubeRenderer(store, resFactory);
    return g_Renderer;
  }

public:
  CGameGlobalObjects(IFactory* resFactory, CSimplePool* objStore)
  : x4_resFactory(resFactory), xcc_simplePool(objStore) {
    if (!x4_resFactory) {
      m_gameResFactory = std::make_unique<CResFactory>();
      x4_resFactory = m_gameResFactory.get();
    }
    if (!xcc_simplePool) {
      m_gameSimplePool = std::make_unique<CSimplePool>(*x4_resFactory);
      xcc_simplePool = m_gameSimplePool.get();
    }
    g_ResFactory = x4_resFactory;
    g_SimplePool = xcc_simplePool;
    g_CharFactoryBuilder = &xec_charFactoryBuilder;
    g_AiFuncMap = &x110_aiFuncMap;
    x134_gameState = std::make_unique<CGameState>();
    g_GameState = x134_gameState.get();
    g_TweakManager = &x150_tweakManager;
  }

  ~CGameGlobalObjects();

  void PostInitialize();

  void ResetGameState() {
    x134_gameState = std::make_unique<CGameState>();
    g_GameState = x134_gameState.get();
  }

  void StreamInGameState(CInputStream& stream, u32 saveIdx) {
    x134_gameState = std::make_unique<CGameState>(stream, saveIdx);
    g_GameState = x134_gameState.get();
  }
};

class CGameArchitectureSupport {
  friend class CMain;
  CMain& m_parent;
  CArchitectureQueue x4_archQueue;
  CAudioSys x0_audioSys;
  CInputGenerator x30_inputGenerator;
  CGuiSys x44_guiSys;
  CIOWinManager x58_ioWinManager;
  s32 x78_gameFrameCount = 0;

  enum class EAudioLoadStatus { Loading, Loaded, Uninitialized };
  EAudioLoadStatus x88_audioLoadStatus = EAudioLoadStatus::Uninitialized;
  std::vector<TToken<CAudioGroupSet>> x8c_pendingAudioGroups;

  aurora::WindowSize m_windowRect;
  bool m_rectIsDirty = false;

  void destroyed() { x4_archQueue.Push(MakeMsg::CreateRemoveAllIOWins(EArchMsgTarget::IOWinManager)); }

  void resized(const aurora::WindowSize& rect) {
    m_windowRect = rect;
    m_rectIsDirty = true;
  }

public:
  CGameArchitectureSupport(CMain& parent, boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend);
  ~CGameArchitectureSupport();

  void mouseDown(const SWindowCoord& coord, EMouseButton button, EModifierKey mods) {
    // x30_inputGenerator.mouseDown(coord, button, mods);
  }
  void mouseUp(const SWindowCoord& coord, EMouseButton button, EModifierKey mods) {
    // x30_inputGenerator.mouseUp(coord, button, mods);
  }
  void mouseMove(const SWindowCoord& coord) {
    // x30_inputGenerator.mouseMove(coord);
  }
  void scroll(const SWindowCoord& coord, const SScrollDelta& scroll) {
    // x30_inputGenerator.scroll(coord, scroll);
  }
  void charKeyDown(uint8_t charCode, aurora::ModifierKey mods, bool isRepeat);
  void charKeyUp(uint8_t charCode, aurora::ModifierKey mods) {
    // x30_inputGenerator.charKeyUp(charCode, mods);
  }
  void specialKeyDown(aurora::SpecialKey key, aurora::ModifierKey mods, bool isRepeat);

  void specialKeyUp(aurora::SpecialKey key, aurora::ModifierKey mods);
  void modKeyDown(aurora::ModifierKey mod, bool isRepeat) {
    // x30_inputGenerator.modKeyDown(mod, isRepeat);
  }
  void modKeyUp(aurora::ModifierKey mod) {
    // x30_inputGenerator.modKeyUp(mod);
  }

  void PreloadAudio();
  bool LoadAudio();
  void UnloadAudio();
  void UpdateTicks(float dt);
  void Update(float dt);
  void Draw();

  bool isRectDirty() const { return m_rectIsDirty; }
  const aurora::WindowSize& getWindowRect() {
    m_rectIsDirty = false;
    return m_windowRect;
  }

  CIOWinManager& GetIOWinManager() { return x58_ioWinManager; }
};

class CMain : public IMain {
  friend class CGameArchitectureSupport;

private:
  // COsContext x0_osContext;
  // CMemorySys x6c_memSys;
  CTweaks x70_tweaks;
  EGameplayResult xe4_gameplayResult;
  double xe8_;
  rstl::reserved_vector<float, 4> xf0_;
  rstl::reserved_vector<float, 4> x104_;
  float x118_;
  float x11c_;
  float x120_;
  float x124_;
  std::unique_ptr<CGameGlobalObjects> x128_globalObjects;
  EClientFlowStates x12c_flowState = EClientFlowStates::Default;
  rstl::reserved_vector<u32, 10> x130_{{
      1000000,
      1000000,
      1000000,
      1000000,
      1000000,
      1000000,
      1000000,
      1000000,
      1000000,
      1000000,
  }};
  // u32 x15c_ = 0;
  bool x160_24_finished : 1 = false;
  bool x160_25_mfGameBuilt : 1 = false;
  bool x160_26_screenFading : 1 = false;
  bool x160_27_ : 1 = false;
  bool x160_28_manageCard : 1 = false;
  bool x160_29_ : 1 = false;
  bool x160_30_ : 1 = false;
  bool x160_31_cardBusy : 1 = false;
  bool x161_24_gameFrameDrawn : 1 = false;
  std::unique_ptr<CGameArchitectureSupport> x164_archSupport;

  CVarManager* m_cvarMgr = nullptr;
  bool m_loadedPersistentResources = false;
  bool m_doQuit = false;
  bool m_paused = false;
  MetaforceVersionInfo m_version;

  void InitializeSubsystems();
  static void InitializeDiscord();
  static void ShutdownDiscord();
  static void HandleDiscordReady(const DiscordUser* request);
  static void HandleDiscordDisconnected(int errorCode, const char* message);
  static void HandleDiscordErrored(int errorCode, const char* message);

public:
  CMain(IFactory* resFactory, CSimplePool* resStore);
  void RegisterResourceTweaks();
  void AddWorldPaks();
  void AddOverridePaks();
  void ResetGameState();
  void StreamNewGameState(CInputStream&, u32 idx);
  void RefreshGameState();
  void CheckTweakManagerDebugOptions() {}
  void SetMFGameBuilt(bool b) { x160_25_mfGameBuilt = b; }
  void SetScreenFading(bool b) { x160_26_screenFading = b; }
  bool GetScreenFading() const { return x160_26_screenFading; }

  static void UpdateDiscordPresence(CAssetId worldSTRG = {});

  // int RsMain(int argc, char** argv, boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator&
  // backend);
  void Init(const FileStoreManager& storeMgr, CVarManager* cvarManager, boo::IAudioVoiceEngine* voiceEngine,
            amuse::IBackendVoiceAllocator& backend) override;
  bool Proc(float dt) override;
  void Draw() override;
  void Shutdown() override;
  //  boo::IWindow* GetMainWindow() const override;

  void MemoryCardInitializePump();

  bool CheckReset() const { return m_doQuit; }
  bool CheckTerminate() const { return m_doQuit; }
  void DrawDebugMetrics(double, CStopwatch&) {}
  void DoPredrawMetrics() {}
  void FillInAssetIDs();
  bool LoadAudio();
  void ShutdownSubsystems();
  EGameplayResult GetGameplayResult() const { return xe4_gameplayResult; }
  void SetGameplayResult(EGameplayResult wl) { xe4_gameplayResult = wl; }
  void SetManageCard(bool v) { x160_28_manageCard = v; }
  bool GetCardBusy() const { return x160_31_cardBusy; }
  void SetCardBusy(bool v) { x160_31_cardBusy = v; }
  void SetGameFrameDrawn() { x161_24_gameFrameDrawn = true; }
  static void EnsureWorldPaksReady();
  static void EnsureWorldPakReady(CAssetId mlvl);

  EClientFlowStates GetFlowState() const override { return x12c_flowState; }
  void SetFlowState(EClientFlowStates s) override { x12c_flowState = s; }

  void SetX30(bool v) { x160_30_ = v; }

  CGameArchitectureSupport* GetArchSupport() const { return x164_archSupport.get(); }

  size_t GetExpectedIdSize() const override { return sizeof(u32); }
  bool IsPAL() const override { return m_version.region == ERegion::PAL; }
  bool IsJapanese() const override { return m_version.region == ERegion::JPN; }
  bool IsUSA() const override { return m_version.region == ERegion::USA; }
  bool IsKorean() const override { return m_version.region == ERegion::KOR; }
  bool IsTrilogy() const override { return m_version.game == EGame::MetroidPrimeTrilogy; }
  ERegion GetRegion() const override { return m_version.region; }
  EGame GetGame() const override { return m_version.game; }
  std::string GetGameTitle() const override { return m_version.gameTitle; }
  std::string_view GetVersionString() const override { return m_version.version; }
  void Quit() override { m_doQuit = true; }
  bool IsPaused() const override { return m_paused; }
  void SetPaused(bool b) override { m_paused = b; }

  int m_warpWorldIdx = -1;
  TAreaId m_warpAreaId = 0;
  u64 m_warpLayerBits = 0;
  std::vector<TEditorId> m_warpMemoryRelays;
};

} // namespace MP1
} // namespace metaforce
