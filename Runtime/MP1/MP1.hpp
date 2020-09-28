#pragma once

#ifndef MP1_USE_BOO
#define MP1_USE_BOO 0
#endif
#ifndef MP1_VARIABLE_DELTA_TIME
#define MP1_VARIABLE_DELTA_TIME 1
#endif

#include "IMain.hpp"
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
#include "CAudioStateWin.hpp"
#include "GameGlobalObjects.hpp"
#include "CArchitectureQueue.hpp"
#include "CTimeProvider.hpp"
#include "GuiSys/CTextExecuteBuffer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "World/CScriptMazeNode.hpp"

struct DiscordUser;

namespace urde {
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
    g_Renderer = new CBooRenderer(store, resFactory);
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

  void PostInitialize() {
    AddPaksAndFactories();
    LoadTextureCache();
    LoadStringTable();
    m_renderer.reset(AllocateRenderer(*xcc_simplePool, *x4_resFactory));
    CEnvFxManager::Initialize();
    CScriptMazeNode::LoadMazeSeeds();
  }

  void ResetGameState() {
    x134_gameState = std::make_unique<CGameState>();
    g_GameState = x134_gameState.get();
  }

  void StreamInGameState(CBitStreamReader& stream, u32 saveIdx) {
    x134_gameState = std::make_unique<CGameState>(stream, saveIdx);
    g_GameState = x134_gameState.get();
  }
};

#if MP1_USE_BOO
class CGameArchitectureSupport : public boo::IWindowCallback
#else
class CGameArchitectureSupport
#endif
{
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

  hsh::rect2d m_windowRect;
  bool m_rectIsDirty;

  void destroyed() { x4_archQueue.Push(MakeMsg::CreateRemoveAllIOWins(EArchMsgTarget::IOWinManager)); }

  void resized(const hsh::rect2d& rect) {
    m_windowRect = rect;
    m_rectIsDirty = true;
  }

public:
  CGameArchitectureSupport(CMain& parent, boo2::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend);
  ~CGameArchitectureSupport();

  void mouseDown(const hsh::offset2dF& offset,
                 boo2::MouseButton button, boo2::KeyModifier mods) {
    x30_inputGenerator.mouseDown(offset, button, mods);
  }
  void mouseUp(const hsh::offset2dF& offset,
               boo2::MouseButton button, boo2::KeyModifier mods) {
    x30_inputGenerator.mouseUp(offset, button, mods);
  }
  void mouseMove(const hsh::offset2dF& offset,
                 boo2::KeyModifier mods) { x30_inputGenerator.mouseMove(offset, mods); }
  void scroll(const hsh::offset2dF& scroll, boo2::KeyModifier mods) {
    x30_inputGenerator.scroll(scroll, mods);
  }
  void charKeyDown(unsigned long charCode, boo2::KeyModifier mods);
  void charKeyUp(unsigned long charCode, boo2::KeyModifier mods) { x30_inputGenerator.charKeyUp(charCode, mods); }
  void specialKeyDown(boo2::Keycode key, boo2::KeyModifier mods);

  void specialKeyUp(boo2::Keycode key, boo2::KeyModifier mods);
  void modKeyDown(boo2::KeyModifier mod) { x30_inputGenerator.modKeyDown(mod); }
  void modKeyUp(boo2::KeyModifier mod) { x30_inputGenerator.modKeyUp(mod); }

  void PreloadAudio();
  bool LoadAudio();
  void UnloadAudio();
  void UpdateTicks(float dt);
  void Update(float dt);
  void Draw();

  bool isRectDirty() const { return m_rectIsDirty; }
  const hsh::rect2d& getWindowRect() {
    m_rectIsDirty = false;
    return m_windowRect;
  }

  CIOWinManager& GetIOWinManager() { return x58_ioWinManager; }
};

#if MP1_USE_BOO
class CMain : public boo::IApplicationCallback,
              public IMain
#else
class CMain : public IMain
#endif
{
  friend class CGameArchitectureSupport;
#if MP1_USE_BOO
  boo::IWindow* mainWindow;
  int appMain(boo::IApplication* app);
  void appQuitting(boo::IApplication*) { xe8_b24_finished = true; }
  void appFilesOpen(boo::IApplication*, const std::vector<std::string>& paths) {
    fmt::print(stderr, FMT_STRING("OPENING: "));
    for (const std::string& path : paths)
      fprintf(stderr, "%s ", path.c_str());
    fprintf(stderr, "\n");
  }
#endif
private:
  // CMemorySys x6c_memSys;
  CTweaks x70_tweaks;
  EGameplayResult xe4_gameplayResult;

  /* urde addition: these are simply initialized along with everything else */
  CGameGlobalObjects x128_globalObjects;

  EFlowState x12c_flowState = EFlowState::Default;

  u32 x130_[10] = {1000000};

  bool x160_24_finished : 1;
  bool x160_25_mfGameBuilt : 1;
  bool x160_26_screenFading : 1;
  bool x160_27_ : 1;
  bool x160_28_manageCard : 1;
  bool x160_29_ : 1;
  bool x160_30_ : 1;
  bool x160_31_cardBusy : 1;
  bool x161_24_gameFrameDrawn : 1;

  std::unique_ptr<CGameArchitectureSupport> x164_archSupport;

  hecl::CVarManager* m_cvarMgr = nullptr;
  // Warmup state
  std::vector<SObjectTag> m_warmupTags;
  std::vector<SObjectTag>::iterator m_warmupIt;
  bool m_needsWarmupClear = false;
  bool m_loadedPersistentResources = false;
  bool m_doQuit = false;

#if MP1_VARIABLE_DELTA_TIME
  bool m_firstFrame = true;
  using delta_clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<delta_clock> m_prevFrameTime;
#endif

  void InitializeSubsystems();
  static void InitializeDiscord();
  static void ShutdownDiscord();
  static void HandleDiscordReady(const DiscordUser* request);
  static void HandleDiscordDisconnected(int errorCode, const char* message);
  static void HandleDiscordErrored(int errorCode, const char* message);

public:
  CMain(IFactory* resFactory, CSimplePool* resStore, hsh::surface surface);
  void RegisterResourceTweaks();
  void AddWorldPaks();
  void AddOverridePaks();
  void ResetGameState();
  void StreamNewGameState(CBitStreamReader&, u32 idx);
  void RefreshGameState();
  void CheckTweakManagerDebugOptions() {}
  void SetMFGameBuilt(bool b) { x160_25_mfGameBuilt = b; }
  void SetScreenFading(bool b) { x160_26_screenFading = b; }
  bool GetScreenFading() const { return x160_26_screenFading; }

  static void UpdateDiscordPresence(CAssetId worldSTRG = {});

  // int RsMain(int argc, const boo::SystemChar* argv[]);
  void Init(const hecl::Runtime::FileStoreManager& storeMgr, hecl::CVarManager* cvarManager,
            boo2::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend) override;
  void WarmupShaders() override;
  bool Proc() override;
  void Draw() override;
  void Shutdown() override;

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

  EFlowState GetFlowState() const override { return x12c_flowState; }
  void SetFlowState(EFlowState s) override { x12c_flowState = s; }

  void SetX30(bool v) { x160_30_ = v; }

  CGameArchitectureSupport* GetArchSupport() const { return x164_archSupport.get(); }

  size_t GetExpectedIdSize() const override { return sizeof(u32); }
#if 0
  void quit(hecl::Console*, const std::vector<std::string>&) { m_doQuit = true; }
  void Give(hecl::Console*, const std::vector<std::string>&);
  void Remove(hecl::Console*, const std::vector<std::string>&);
  void God(hecl::Console*, const std::vector<std::string>&);
  void Teleport(hecl::Console*, const std::vector<std::string>&);
  void ListWorlds(hecl::Console*, const std::vector<std::string>&);
  void Warp(hecl::Console*, const std::vector<std::string>&);
#endif

  int m_warpWorldIdx = -1;
  TAreaId m_warpAreaId = 0;
  u64 m_warpLayerBits = 0;
  std::vector<TEditorId> m_warpMemoryRelays;
};

} // namespace MP1
} // namespace urde
