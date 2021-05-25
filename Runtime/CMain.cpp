#include <string>
#include <string_view>
#include <numeric>
#include <hecl/Pipeline.hpp>

#include "boo/boo.hpp"
#include "logvisor/logvisor.hpp"

#include "ImGuiEngine.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/MP1/MP1.hpp"
#include "amuse/BooBackend.hpp"

#include "../version.h"

/* Static reference to dataspec additions
 * (used by MSVC to definitively link DataSpecs) */
#include "DataSpecRegistry.hpp"

using namespace std::literals;

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file, const char*, int line, fmt::string_view fmt,
                      fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

class Limiter {
  using delta_clock = std::chrono::steady_clock;
  using nanotime_t = std::chrono::nanoseconds::rep;

public:
  void Sleep(nanotime_t targetFrameTime) {
    if (targetFrameTime == 0) {
      return;
    }

    auto start = delta_clock::now();
    nanotime_t adjustedSleepTime = ShouldSleep(targetFrameTime);
    if (adjustedSleepTime > 0) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(adjustedSleepTime));
      nanotime_t overslept = TimeSince(start) - adjustedSleepTime;
      if (overslept < targetFrameTime) {
        m_overheadTimes[m_overheadTimeIdx] = overslept;
        m_overheadTimeIdx = (m_overheadTimeIdx + 1) % m_overheadTimes.size();
      }
    }
    m_oldTime = delta_clock::now();
  }

  nanotime_t ShouldSleep(nanotime_t targetFrameTime) {
    nanotime_t sleepTime = targetFrameTime - TimeSince(m_oldTime);
    m_overhead = std::accumulate(m_overheadTimes.begin(), m_overheadTimes.end(), nanotime_t{}) /
                 static_cast<nanotime_t>(m_overheadTimes.size());
    if (sleepTime > m_overhead) {
      return sleepTime - m_overhead;
    }
    return 0;
  }

private:
  delta_clock::time_point m_oldTime;
  std::array<nanotime_t, 4> m_overheadTimes{};
  size_t m_overheadTimeIdx = 0;
  nanotime_t m_overhead = 0;

  nanotime_t TimeSince(delta_clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(delta_clock::now() - start).count();
  }
};

extern hecl::SystemString ExeDir;

namespace metaforce {
static logvisor::Module Log{"Metaforce"};

std::optional<MP1::CMain> g_mainMP1;

static hecl::SystemString CPUFeatureString(const zeus::CPUInfo& cpuInf) {
  hecl::SystemString features;
#if defined(__x86_64__) || defined(_M_X64)
  auto AddFeature = [&features](const hecl::SystemChar* str) {
    if (!features.empty())
      features += _SYS_STR(", ");
    features += str;
  };
  if (cpuInf.AESNI)
    AddFeature(_SYS_STR("AES-NI"));
  if (cpuInf.SSE1)
    AddFeature(_SYS_STR("SSE"));
  if (cpuInf.SSE2)
    AddFeature(_SYS_STR("SSE2"));
  if (cpuInf.SSE3)
    AddFeature(_SYS_STR("SSE3"));
  if (cpuInf.SSSE3)
    AddFeature(_SYS_STR("SSSE3"));
  if (cpuInf.SSE4a)
    AddFeature(_SYS_STR("SSE4a"));
  if (cpuInf.SSE41)
    AddFeature(_SYS_STR("SSE4.1"));
  if (cpuInf.SSE42)
    AddFeature(_SYS_STR("SSE4.2"));
  if (cpuInf.AVX)
    AddFeature(_SYS_STR("AVX"));
  if (cpuInf.AVX2)
    AddFeature(_SYS_STR("AVX2"));
#endif
  return features;
}

struct WindowCallback : boo::IWindowCallback {
  friend struct Application;

private:
  bool m_fullscreenToggleRequested = false;
  boo::SWindowRect m_lastRect;
  bool m_rectDirty = false;
  bool m_windowInvalid = false;
  ImGuiWindowCallback m_imguiCallback;

  void resized(const boo::SWindowRect& rect, bool sync) override {
    m_lastRect = rect;
    m_rectDirty = true;
    m_imguiCallback.resized(rect, sync);
  }

  void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) override {
    if (!ImGuiWindowCallback::m_mouseCaptured && g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->mouseDown(coord, button, mods);
      }
    }
    m_imguiCallback.mouseDown(coord, button, mods);
  }

  void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) override {
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->mouseUp(coord, button, mods);
      }
    }
    m_imguiCallback.mouseUp(coord, button, mods);
  }

  void mouseMove(const boo::SWindowCoord& coord) override {
    if (!ImGuiWindowCallback::m_mouseCaptured && g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->mouseMove(coord);
      }
    }
    m_imguiCallback.mouseMove(coord);
  }

  void mouseEnter(const boo::SWindowCoord& coord) override { m_imguiCallback.mouseEnter(coord); }

  void mouseLeave(const boo::SWindowCoord& coord) override { m_imguiCallback.mouseLeave(coord); }

  void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll) override {
    if (!ImGuiWindowCallback::m_mouseCaptured && g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->scroll(coord, scroll);
      }
    }
    m_imguiCallback.scroll(coord, scroll);
  }

  void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat) override {
    if (!ImGuiWindowCallback::m_keyboardCaptured && g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->charKeyDown(charCode, mods, isRepeat);
      }
    }
    m_imguiCallback.charKeyDown(charCode, mods, isRepeat);
  }

  void charKeyUp(unsigned long charCode, boo::EModifierKey mods) override {
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->charKeyUp(charCode, mods);
      }
    }
    m_imguiCallback.charKeyUp(charCode, mods);
  }

  void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat) override {
    if (!ImGuiWindowCallback::m_keyboardCaptured && g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->specialKeyDown(key, mods, isRepeat);
      }
    }
    if (key == boo::ESpecialKey::Enter && True(mods & boo::EModifierKey::Alt)) {
      m_fullscreenToggleRequested = true;
    }
    m_imguiCallback.specialKeyDown(key, mods, isRepeat);
  }

  void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods) override {
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->specialKeyUp(key, mods);
      }
    }
    m_imguiCallback.specialKeyUp(key, mods);
  }

  void destroyed() override { m_windowInvalid = true; }
};

struct Application : boo::IApplicationCallback {
private:
  std::shared_ptr<boo::IWindow> m_window;
  WindowCallback m_windowCallback;
  hecl::Runtime::FileStoreManager& m_fileMgr;
  hecl::CVarManager& m_cvarManager;
  hecl::CVarCommons& m_cvarCommons;

  boo::ObjToken<boo::ITextureR> m_renderTex;
  hecl::SystemString m_deferredProject;
  std::unique_ptr<hecl::Database::Project> m_proj;
  std::optional<amuse::BooBackendVoiceAllocator> m_amuseAllocWrapper;
  std::unique_ptr<boo::IAudioVoiceEngine> m_voiceEngine;
  std::unique_ptr<hecl::PipelineConverterBase> m_pipelineConv;

  Limiter m_limiter{};
  std::atomic_bool m_running = {true};
  bool m_noShaderWarmup = false;
  bool m_imGuiInitialized = false;

  bool m_firstFrame = true;
  using delta_clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<delta_clock> m_prevFrameTime;

public:
  Application(hecl::Runtime::FileStoreManager& fileMgr, hecl::CVarManager& cvarMgr, hecl::CVarCommons& cvarCmns)
  : m_fileMgr(fileMgr), m_cvarManager(cvarMgr), m_cvarCommons(cvarCmns) {}

  int appMain(boo::IApplication* app) override {
    initialize(app);

    m_window = app->newWindow(_SYS_STR("Metaforce"sv));
    if (!m_window) {
      return 1;
    }
    m_window->setCallback(&m_windowCallback);
    m_window->showWindow();

    boo::SWindowRect rect = m_window->getWindowFrame();
    m_windowCallback.m_lastRect = rect;
    boo::IGraphicsDataFactory* gfxF = m_window->getMainContextDataFactory();
    gfxF->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) {
      m_renderTex = ctx.newRenderTexture(rect.size[0], rect.size[1], boo::TextureClampMode::ClampToEdge, 3, 3);
      return true;
    } BooTrace);

    m_pipelineConv = hecl::NewPipelineConverter(gfxF);
    hecl::conv = m_pipelineConv.get();

    m_voiceEngine = boo::NewAudioVoiceEngine();
    m_voiceEngine->setVolume(0.7f);
    m_amuseAllocWrapper.emplace(*m_voiceEngine);

    hecl::ProjectPath projectPath;
    for (const auto& arg : app->getArgs()) {
      hecl::Sstat theStat;
      if (!hecl::Stat((arg + _SYS_STR("/out")).c_str(), &theStat) && S_ISDIR(theStat.st_mode)) {
        hecl::ProjectRootPath rootPath(arg);
        hecl::Database::Project tmp(rootPath); // Force project creation
      }
      if (m_deferredProject.empty() && hecl::SearchForProject(arg))
        m_deferredProject = arg;
      if (arg == _SYS_STR("--no-shader-warmup"))
        m_noShaderWarmup = true;
      else if (arg == _SYS_STR("--no-sound"))
        m_voiceEngine->setVolume(0.f);
    }

    if (m_deferredProject.empty()) {
      /* Default behavior - search upwards for packaged project containing the program */
      if (hecl::ProjectRootPath projRoot = hecl::SearchForProject(ExeDir)) {
        hecl::SystemString rootPath(projRoot.getAbsolutePath());
        hecl::Sstat theStat;
        if (hecl::Stat((rootPath + _SYS_STR("/out/files/MP1/Metroid1.upak")).c_str(), &theStat) == 0 &&
            S_ISREG(theStat.st_mode))
          m_deferredProject = rootPath + _SYS_STR("/out");
      }
    }

    while (m_running) {
      onAppIdle();
    }

    if (m_imGuiInitialized) {
      ImGuiEngine::Shutdown();
    }
    if (g_mainMP1) {
      g_mainMP1->Shutdown();
    }
    g_mainMP1.reset();
    if (m_window) {
      m_window->getCommandQueue()->stopRenderer();
    }
    m_voiceEngine.reset();
    m_amuseAllocWrapper.reset();
    CDvdFile::Shutdown();
    return 0;
  }

  void initialize(boo::IApplication* app) {
    zeus::detectCPU();

    for (const boo::SystemString& arg : app->getArgs()) {
      if (arg.find(_SYS_STR("--verbosity=")) == 0 || arg.find(_SYS_STR("-v=")) == 0) {
        hecl::SystemUTF8Conv utf8Arg(arg.substr(arg.find_last_of('=') + 1));
        hecl::VerbosityLevel = atoi(utf8Arg.c_str());
        hecl::LogModule.report(logvisor::Info, FMT_STRING("Set verbosity level to {}"), hecl::VerbosityLevel);
      }
    }

    const zeus::CPUInfo& cpuInf = zeus::cpuFeatures();
    Log.report(logvisor::Info, FMT_STRING("CPU Name: {}"), cpuInf.cpuBrand);
    Log.report(logvisor::Info, FMT_STRING("CPU Vendor: {}"), cpuInf.cpuVendor);
    Log.report(logvisor::Info, FMT_STRING(_SYS_STR("CPU Features: {}")), CPUFeatureString(cpuInf));
  }

  void onAppIdle() noexcept {
    if (!m_deferredProject.empty()) {
      hecl::SystemString subPath;
      hecl::ProjectRootPath projPath = hecl::SearchForProject(m_deferredProject, subPath);
      if (!projPath) {
        Log.report(logvisor::Error, FMT_STRING(_SYS_STR("project doesn't exist at '{}'")), m_deferredProject);
        m_running.store(false);
        return;
      }
      m_proj = std::make_unique<hecl::Database::Project>(projPath);
      m_deferredProject.clear();
      hecl::ProjectPath projectPath{m_proj->getProjectWorkingPath(), _SYS_STR("out/files/MP1")};
      CDvdFile::Initialize(projectPath);
    }
    if (!m_proj) {
      Log.report(logvisor::Error, FMT_STRING(_SYS_STR("Project directory not specified")));
      m_running.store(false);
      return;
    }

    m_cvarManager.proc();

    if (m_windowCallback.m_windowInvalid) {
      m_running.store(false);
      return;
    }

    m_window->waitForRetrace();

    boo::SWindowRect rect = m_windowCallback.m_lastRect;
    rect.location = {0, 0};
    boo::IGraphicsCommandQueue* gfxQ = m_window->getCommandQueue();
    if (m_windowCallback.m_rectDirty) {
      gfxQ->resizeRenderTexture(m_renderTex, rect.size[0], rect.size[1]);
      CGraphics::SetViewportResolution({rect.size[0], rect.size[1]});
      m_windowCallback.m_rectDirty = false;
    } else if (m_firstFrame) {
      CGraphics::SetViewportResolution({rect.size[0], rect.size[1]});
    }

    if (m_windowCallback.m_fullscreenToggleRequested) {
      m_window->setFullscreen(!m_window->isFullscreen());
      m_windowCallback.m_fullscreenToggleRequested = false;
    }

    boo::IGraphicsDataFactory* gfxF = m_window->getMainContextDataFactory();
    float scale = std::floor(m_window->getVirtualPixelFactor() * 4.f) / 4.f;
    if (!g_mainMP1) {
      g_mainMP1.emplace(nullptr, nullptr, gfxF, gfxQ, m_renderTex.get());
      g_mainMP1->Init(m_fileMgr, &m_cvarManager, m_window.get(), m_voiceEngine.get(), *m_amuseAllocWrapper);
      if (!m_noShaderWarmup) {
        g_mainMP1->WarmupShaders();
      }
      ImGuiEngine::Initialize(gfxF, m_window.get(), scale);
      m_imGuiInitialized = true;
    }

    float dt = 1 / 60.f;
    float realDt = dt;
    auto now = delta_clock::now();
    if (m_firstFrame) {
      m_firstFrame = false;
    } else {
      using delta_duration = std::chrono::duration<float, std::ratio<1>>;
      realDt = std::chrono::duration_cast<delta_duration>(now - m_prevFrameTime).count();
      if (m_cvarCommons.m_variableDt->toBoolean()) {
        dt = std::min(realDt, 1 / 30.f);
      }
    }
    m_prevFrameTime = now;

    ImGuiEngine::Begin(realDt, scale);

    if (g_mainMP1->Proc(dt)) {
      m_running.store(false);
      return;
    }

    {
      OPTICK_EVENT("Draw");
      gfxQ->setRenderTarget(m_renderTex);
      gfxQ->setViewport(rect);
      gfxQ->setScissor(rect);
      if (g_Renderer != nullptr) {
        g_Renderer->BeginScene();
      }
      g_mainMP1->Draw();
      if (g_Renderer != nullptr) {
        g_Renderer->EndScene();
      }
    }

    {
      OPTICK_EVENT("ImGui Draw");
      ImGuiEngine::End();
      ImGuiEngine::Draw(gfxQ);
    }

    {
      OPTICK_EVENT("Execute");
      gfxQ->execute();
    }

    gfxQ->resolveDisplay(m_renderTex);

    if (g_ResFactory != nullptr) {
      int64_t targetFrameTime = getTargetFrameTime();
      do {
        g_ResFactory->AsyncIdle();
      } while (m_limiter.ShouldSleep(targetFrameTime) != 0);
      m_limiter.Sleep(targetFrameTime);
    }

    if (m_voiceEngine) {
      m_voiceEngine->pumpAndMixVoices();
    }
    CBooModel::ClearModelUniformCounters();
    CGraphics::TickRenderTimings();
    ++logvisor::FrameIndex;
  }

  void appQuitting(boo::IApplication* /*unused*/) override { m_running.store(false); }

  [[nodiscard]] std::string getGraphicsApi() const { return m_cvarCommons.getGraphicsApi(); }

  [[nodiscard]] uint32_t getSamples() const { return m_cvarCommons.getSamples(); }

  [[nodiscard]] uint32_t getAnisotropy() const { return m_cvarCommons.getAnisotropy(); }

  [[nodiscard]] bool getDeepColor() const { return m_cvarCommons.getDeepColor(); }

  [[nodiscard]] int64_t getTargetFrameTime() const {
    return m_cvarCommons.getVariableFrameTime() ? 0 : 1000000000L / 60;
  }
};

} // namespace metaforce

static hecl::SystemChar CwdBuf[1024];
hecl::SystemString ExeDir;

static void SetupBasics(bool logging) {
  auto result = zeus::validateCPU();
  if (!result.first) {
#if _WIN32 && !WINDOWS_STORE
    std::wstring msg =
        fmt::format(FMT_STRING(L"ERROR: This build of Metaforce requires the following CPU features:\n{}\n"),
                    metaforce::CPUFeatureString(result.second));
    MessageBoxW(nullptr, msg.c_str(), L"CPU error", MB_OK | MB_ICONERROR);
#else
    fmt::print(stderr, FMT_STRING("ERROR: This build of Metaforce requires the following CPU features:\n{}\n"),
               metaforce::CPUFeatureString(result.second));
#endif
    exit(1);
  }

  logvisor::RegisterStandardExceptions();
  if (logging)
    logvisor::RegisterConsoleLogger();
  atSetExceptionHandler(AthenaExc);

#if SENTRY_ENABLED
  hecl::Runtime::FileStoreManager fileMgr{_SYS_STR("sentry-native-metaforce")};
  hecl::SystemUTF8Conv cacheDir{fileMgr.getStoreRoot()};
  logvisor::RegisterSentry("metaforce", METAFORCE_WC_DESCRIBE, cacheDir.c_str());
#endif
}

static bool IsClientLoggingEnabled(int argc, const boo::SystemChar** argv) {
  for (int i = 1; i < argc; ++i)
    if (!hecl::StrNCmp(argv[i], _SYS_STR("-l"), 2))
      return true;
  return false;
}

#if !WINDOWS_STORE
#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
  if (argc > 1 && !hecl::StrCmp(argv[1], _SYS_STR("--dlpackage"))) {
    fmt::print(FMT_STRING("{}\n"), METAFORCE_DLPACKAGE);
    return 100;
  }

  SetupBasics(IsClientLoggingEnabled(argc, argv));
  hecl::Runtime::FileStoreManager fileMgr{_SYS_STR("metaforce")};
  hecl::CVarManager cvarMgr{fileMgr};
  hecl::CVarCommons cvarCmns{cvarMgr};

  std::vector<boo::SystemString> args;
  for (int i = 1; i < argc; ++i)
    args.push_back(argv[i]);
  cvarMgr.parseCommandLine(args);

  hecl::SystemString logFile{hecl::SystemStringConv(cvarCmns.getLogFile()).c_str()};
  hecl::SystemString logFilePath;
  if (!logFile.empty()) {
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[100];
    std::strftime(buf, 100, "%Y-%m-%d_%H-%M-%S", std::localtime(&time));
    hecl::SystemString timeStr = hecl::SystemStringConv(buf).c_str();
    logFilePath = fmt::format(FMT_STRING(_SYS_STR("{}/{}-{}")), fileMgr.getStoreRoot(), timeStr, logFile);
    logvisor::RegisterFileLogger(logFilePath.c_str());
  }

  if (hecl::SystemChar* cwd = hecl::Getcwd(CwdBuf, 1024)) {
    if (hecl::PathRelative(argv[0]))
      ExeDir = hecl::SystemString(cwd) + _SYS_STR('/');
    hecl::SystemString Argv0(argv[0]);
    hecl::SystemString::size_type lastIdx = Argv0.find_last_of(_SYS_STR("/\\"));
    if (lastIdx != hecl::SystemString::npos)
      ExeDir.insert(ExeDir.end(), Argv0.begin(), Argv0.begin() + lastIdx);
  }

  /* Handle -j argument */
  hecl::SetCpuCountOverride(argc, argv);

  metaforce::Application appCb(fileMgr, cvarMgr, cvarCmns);
  int ret = boo::ApplicationRun(boo::IApplication::EPlatformType::Auto, appCb, _SYS_STR("metaforce"),
                                _SYS_STR("Metaforce"), argc, argv, appCb.getGraphicsApi(), appCb.getSamples(),
                                appCb.getAnisotropy(), appCb.getDeepColor(), false);
  return ret;
}
#endif

#if WINDOWS_STORE
#include "boo/UWPViewProvider.hpp"
using namespace Windows::ApplicationModel::Core;

[Platform::MTAThread] int WINAPIV main(Platform::Array<Platform::String ^> ^ params) {
  SetupBasics(false);
  metaforce::Application appCb;
  auto viewProvider = ref new boo::ViewProvider(appCb, _SYS_STR("metaforce"), _SYS_STR("Metaforce"),
                                                _SYS_STR("metaforce"), params, false);
  CoreApplication::Run(viewProvider);
  return 0;
}

#elif _WIN32
#include <shellapi.h>

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int) {
  int argc = 0;
  const boo::SystemChar** argv;
  if (lpCmdLine[0])
    argv = (const wchar_t**)(CommandLineToArgvW(lpCmdLine, &argc));
  static boo::SystemChar selfPath[1024];
  GetModuleFileNameW(nullptr, selfPath, 1024);
  static const boo::SystemChar* booArgv[32] = {};
  booArgv[0] = selfPath;
  for (int i = 0; i < argc; ++i)
    booArgv[i + 1] = argv[i];

  const DWORD outType = GetFileType(GetStdHandle(STD_ERROR_HANDLE));
  if (IsClientLoggingEnabled(argc + 1, booArgv) && outType == FILE_TYPE_UNKNOWN)
    logvisor::CreateWin32Console();
  return wmain(argc + 1, booArgv);
}
#endif