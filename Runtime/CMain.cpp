#include <string>
#include <string_view>
#include <numeric>
#include <iostream>

#include "logvisor/logvisor.hpp"

#include "ImGuiEngine.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/MP1/MP1.hpp"
#include "amuse/BooBackend.hpp"

#include "../version.h"

//#include <fenv.h>
//#pragma STDC FENV_ACCESS ON

/* Static reference to dataspec additions
 * (used by MSVC to definitively link DataSpecs) */
#include "DataSpecRegistry.hpp"

#include "Graphics/include/aurora.hpp"
#include "aurora.h"
#include "aurora_shaders.h"

using namespace std::literals;

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file, const char*, int line, fmt::string_view fmt,
                      fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

class Limiter {
  using delta_clock = std::chrono::high_resolution_clock;
  using duration_t = std::chrono::nanoseconds;

public:
  void Reset() { m_oldTime = delta_clock::now(); }

  void Sleep(duration_t targetFrameTime) {
    if (targetFrameTime.count() == 0) {
      return;
    }

    auto start = delta_clock::now();
    duration_t adjustedSleepTime = SleepTime(targetFrameTime);
    if (adjustedSleepTime.count() > 0) {
      NanoSleep(adjustedSleepTime);
      duration_t overslept = TimeSince(start) - adjustedSleepTime;
      if (overslept < duration_t{targetFrameTime}) {
        m_overheadTimes[m_overheadTimeIdx] = overslept;
        m_overheadTimeIdx = (m_overheadTimeIdx + 1) % m_overheadTimes.size();
      }
    }
    Reset();
  }

  duration_t SleepTime(duration_t targetFrameTime) {
    const auto sleepTime = duration_t{targetFrameTime} - TimeSince(m_oldTime);
    m_overhead = std::accumulate(m_overheadTimes.begin(), m_overheadTimes.end(), duration_t{}) / m_overheadTimes.size();
    if (sleepTime > m_overhead) {
      return sleepTime - m_overhead;
    }
    return duration_t{0};
  }

private:
  delta_clock::time_point m_oldTime;
  std::array<duration_t, 4> m_overheadTimes{};
  size_t m_overheadTimeIdx = 0;
  duration_t m_overhead = duration_t{0};

  duration_t TimeSince(delta_clock::time_point start) {
    return std::chrono::duration_cast<duration_t>(delta_clock::now() - start);
  }

#if _WIN32
  bool m_initialized;
  double m_countPerNs;

  void NanoSleep(const duration_t duration) {
    if (!m_initialized) {
      LARGE_INTEGER freq;
      QueryPerformanceFrequency(&freq);
      m_countPerNs = static_cast<double>(freq.QuadPart) / 1000000000.0;
      m_initialized = true;
    }

    DWORD ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    auto tickCount = static_cast<LONGLONG>(static_cast<double>(duration.count()) * m_countPerNs);
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    if (ms > 10) {
      // Adjust for Sleep overhead
      ::Sleep(ms - 10);
    }
    auto end = count.QuadPart + tickCount;
    do {
      QueryPerformanceCounter(&count);
    } while (count.QuadPart < end);
  }
#else
  void NanoSleep(const duration_t duration) { std::this_thread::sleep_for(duration); }
#endif
};

extern std::string ExeDir;

namespace metaforce {
static logvisor::Module Log{"Metaforce"};

std::optional<MP1::CMain> g_mainMP1;

static std::string CPUFeatureString(const zeus::CPUInfo& cpuInf) {
  std::string features;
#if defined(__x86_64__) || defined(_M_X64)
  auto AddFeature = [&features](const char* str) {
    if (!features.empty())
      features += ", ";
    features += str;
  };
  if (cpuInf.AESNI)
    AddFeature("AES-NI");
  if (cpuInf.SSE1)
    AddFeature("SSE");
  if (cpuInf.SSE2)
    AddFeature("SSE2");
  if (cpuInf.SSE3)
    AddFeature("SSE3");
  if (cpuInf.SSSE3)
    AddFeature("SSSE3");
  if (cpuInf.SSE4a)
    AddFeature("SSE4a");
  if (cpuInf.SSE41)
    AddFeature("SSE4.1");
  if (cpuInf.SSE42)
    AddFeature("SSE4.2");
  if (cpuInf.AVX)
    AddFeature("AVX");
  if (cpuInf.AVX2)
    AddFeature("AVX2");
#endif
  return features;
}
#if 0
struct WindowCallback : boo::IWindowCallback {
  friend struct Application;

private:
  bool m_fullscreenToggleRequested = false;
  boo::SWindowRect m_lastRect;
  bool m_rectDirty = false;
  bool m_windowInvalid = false;
  //  ImGuiWindowCallback m_imguiCallback;

  void resized(const boo::SWindowRect& rect, bool sync) override {
    m_lastRect = rect;
    m_rectDirty = true;
    //    m_imguiCallback.resized(rect, sync);
  }

  void mouseDown(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) override {
    //    if (!ImGuiWindowCallback::m_mouseCaptured && g_mainMP1) {
    //      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
    //        as->mouseDown(coord, button, mods);
    //      }
    //    }
    //    m_imguiCallback.mouseDown(coord, button, mods);
  }

  void mouseUp(const boo::SWindowCoord& coord, boo::EMouseButton button, boo::EModifierKey mods) override {
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->mouseUp(coord, button, mods);
      }
    }
    //    m_imguiCallback.mouseUp(coord, button, mods);
  }

  void mouseMove(const boo::SWindowCoord& coord) override {
    //    if (!ImGuiWindowCallback::m_mouseCaptured && g_mainMP1) {
    //      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
    //        as->mouseMove(coord);
    //      }
    //    }
    //    m_imguiCallback.mouseMove(coord);
  }

  //  void mouseEnter(const boo::SWindowCoord& coord) override { m_imguiCallback.mouseEnter(coord); }

  //  void mouseLeave(const boo::SWindowCoord& coord) override { m_imguiCallback.mouseLeave(coord); }

  void scroll(const boo::SWindowCoord& coord, const boo::SScrollDelta& scroll) override {
    //    if (!ImGuiWindowCallback::m_mouseCaptured && g_mainMP1) {
    //      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
    //        as->scroll(coord, scroll);
    //      }
    //    }
    //    m_imguiCallback.scroll(coord, scroll);
  }

  void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat) override {
    //    if (!ImGuiWindowCallback::m_keyboardCaptured && g_mainMP1) {
    //      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
    //        as->charKeyDown(charCode, mods, isRepeat);
    //      }
    //    }
    //    m_imguiCallback.charKeyDown(charCode, mods, isRepeat);
  }

  void charKeyUp(unsigned long charCode, boo::EModifierKey mods) override {
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->charKeyUp(charCode, mods);
      }
    }
    //    m_imguiCallback.charKeyUp(charCode, mods);
  }

  void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat) override {
    //    if (!ImGuiWindowCallback::m_keyboardCaptured && g_mainMP1) {
    //      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
    //        as->specialKeyDown(key, mods, isRepeat);
    //      }
    //    }
    //    if (True(mods & boo::EModifierKey::Alt)) {
    //      if (key == boo::ESpecialKey::Enter) {
    //        m_fullscreenToggleRequested = true;
    //      } else if (key == boo::ESpecialKey::F4) {
    //        m_windowInvalid = true;
    //      }
    //    }
    //    m_imguiCallback.specialKeyDown(key, mods, isRepeat);
  }

  void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods) override {
    //    if (g_mainMP1) {
    //      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
    //        as->specialKeyUp(key, mods);
    //      }
    //    }
    //    m_imguiCallback.specialKeyUp(key, mods);
  }

  void destroyed() override { m_windowInvalid = true; }
};
#endif

struct Application : aurora::AppDelegate {
private:
  hecl::Runtime::FileStoreManager& m_fileMgr;
  hecl::CVarManager& m_cvarManager;
  hecl::CVarCommons& m_cvarCommons;
  ImGuiConsole m_imGuiConsole;
  std::string m_errorString;

  std::string m_deferredProject;
  bool m_projectInitialized = false;
  std::optional<amuse::BooBackendVoiceAllocator> m_amuseAllocWrapper;
  std::unique_ptr<boo::IAudioVoiceEngine> m_voiceEngine;

  Limiter m_limiter{};
  bool m_noShaderWarmup = false;

  bool m_firstFrame = true;
  bool m_fullscreenToggleRequested = false;
  bool m_quitRequested = false;
  using delta_clock = std::chrono::high_resolution_clock;
  std::chrono::time_point<delta_clock> m_prevFrameTime;

  std::vector<u32> m_defferredControllers; // used to capture controllers added before CInputGenerator
                                           // is built, i.e during initialization

public:
  Application(hecl::Runtime::FileStoreManager& fileMgr, hecl::CVarManager& cvarMgr, hecl::CVarCommons& cvarCmns)
  : m_fileMgr(fileMgr), m_cvarManager(cvarMgr), m_cvarCommons(cvarCmns), m_imGuiConsole(cvarMgr, cvarCmns) {}

  void onAppLaunched() override {
    initialize();

    auto backend = static_cast<std::string>(aurora::get_backend_string());
    aurora::set_window_title(fmt::format(FMT_STRING("Metaforce {} [{}]"), METAFORCE_WC_DESCRIBE, backend));

    m_voiceEngine = boo::NewAudioVoiceEngine("metaforce", "Metaforce");
    m_voiceEngine->setVolume(0.7f);
    m_amuseAllocWrapper.emplace(*m_voiceEngine);

    for (const auto& str : aurora::get_args()) {
      auto arg = static_cast<std::string>(str);
      if (m_deferredProject.empty() && !arg.starts_with('-') && !arg.starts_with('+'))
        m_deferredProject = arg;
      if (arg == "--no-shader-warmup")
        m_noShaderWarmup = true;
      else if (arg == "--no-sound")
        m_voiceEngine->setVolume(0.f);
    }
  }

  void initialize() {
    zeus::detectCPU();

    for (const auto& str : aurora::get_args()) {
      auto arg = static_cast<std::string>(str);
      if (arg.starts_with("--verbosity=") || arg.starts_with("-v=")) {
        hecl::VerbosityLevel = atoi(arg.substr(arg.find_last_of('=') + 1).c_str());
        hecl::LogModule.report(logvisor::Info, FMT_STRING("Set verbosity level to {}"), hecl::VerbosityLevel);
      }
    }

    const zeus::CPUInfo& cpuInf = zeus::cpuFeatures();
    Log.report(logvisor::Info, FMT_STRING("CPU Name: {}"), cpuInf.cpuBrand);
    Log.report(logvisor::Info, FMT_STRING("CPU Vendor: {}"), cpuInf.cpuVendor);
    Log.report(logvisor::Info, FMT_STRING("CPU Features: {}"), CPUFeatureString(cpuInf));
  }

  bool onAppIdle(float realDt) noexcept override {
    if (auto* input = g_InputGenerator) {
      if (!m_defferredControllers.empty()) {
        for (const auto which : m_defferredControllers) {
          input->controllerAdded(which);
        }
        m_defferredControllers.clear();
      }
    }

    if (!m_projectInitialized && !m_deferredProject.empty()) {
      if (CDvdFile::Initialize(m_deferredProject)) {
        m_projectInitialized = true;
      } else {
        Log.report(logvisor::Error, FMT_STRING("Project doesn't exist at '{}'"), m_deferredProject);
        m_errorString = fmt::format(FMT_STRING("Project not found at '{}'"), m_deferredProject);
        m_deferredProject.clear();
      }
    }
    if (!m_projectInitialized && m_errorString.empty()) {
      m_errorString = "Project directory not specified"s;
    }

    const auto targetFrameTime = getTargetFrameTime();
    bool skipRetrace = false;
    if (g_ResFactory != nullptr) {
      OPTICK_EVENT("Async Load Resources");
      const auto idleTime = m_limiter.SleepTime(targetFrameTime);
      skipRetrace = g_ResFactory->AsyncIdle(idleTime);
    }

    if (skipRetrace) {
      // We stopped loading resources to catch the next frame
      m_limiter.Reset();
    } else {
      // No more to load, and we're under frame time
      {
        OPTICK_EVENT("Sleep");
        m_limiter.Sleep(targetFrameTime);
      }
    }

    OPTICK_FRAME("MainThread");

    // Check if fullscreen has been toggled, if so set the fullscreen cvar accordingly
    if (m_fullscreenToggleRequested) {
      m_cvarCommons.m_fullscreen->fromBoolean(!m_cvarCommons.getFullscreen());
      m_fullscreenToggleRequested = false;
    }

    // Check if the user has modified the fullscreen CVar, if so set fullscreen state accordingly
    if (m_cvarCommons.m_fullscreen->isModified()) {
      aurora::set_fullscreen(m_cvarCommons.getFullscreen());
    }

    // Let CVarManager inform all CVar listeners of the CVar's state and clear all mdoified flags if necessary
    m_cvarManager.proc();

    if (!g_mainMP1 && m_projectInitialized) {
      g_mainMP1.emplace(nullptr, nullptr);
      g_mainMP1->Init(m_fileMgr, &m_cvarManager, m_voiceEngine.get(), *m_amuseAllocWrapper);
      if (!m_noShaderWarmup) {
        g_mainMP1->WarmupShaders();
      }
    }

    float dt = 1 / 60.f;
    if (m_cvarCommons.m_variableDt->toBoolean()) {
      dt = std::min(realDt, 1 / 30.f);
    }

    if (g_mainMP1) {
      m_imGuiConsole.PreUpdate();
      if (g_mainMP1->Proc(dt)) {
        return false;
      }
      m_imGuiConsole.PostUpdate();
    } else {
      m_imGuiConsole.ShowAboutWindow(false, m_errorString);
    }


    if (m_quitRequested) {
      if (g_mainMP1) {
        g_mainMP1->Quit();
      } else {
        return false;
      }
    }
    return true;
  }

  void onAppDraw() noexcept override {
    OPTICK_EVENT("Draw");
    if (g_Renderer != nullptr) {
      g_Renderer->BeginScene();
      if (g_mainMP1) {
        g_mainMP1->Draw();
      }
      g_Renderer->EndScene();
    }
  }

  void onAppPostDraw() noexcept override {
    OPTICK_EVENT("PostDraw");
    if (m_voiceEngine) {
      m_voiceEngine->pumpAndMixVoices();
    }
    CBooModel::ClearModelUniformCounters();
    CGraphics::TickRenderTimings();
    ++logvisor::FrameIndex;
  }

  void onAppWindowResized(const aurora::WindowSize& size) noexcept override {
    CGraphics::SetViewportResolution({static_cast<s32>(size.width), static_cast<s32>(size.height)});
  }

  void onAppWindowMoved(std::int32_t x, std::int32_t y) noexcept override {
    // TODO: implement this
  }

  void onControllerButton(uint32_t idx, aurora::ControllerButton button, bool pressed) noexcept override {
    if (auto* input = g_InputGenerator) {
      input->controllerButton(idx, button, pressed);
    }
  }

  void onControllerAxis(uint32_t idx, aurora::ControllerAxis axis, int16_t value) noexcept override {
    if (auto* input = g_InputGenerator) {
      input->controllerAxis(idx, axis, value);
    }
  }

  void onControllerAdded(uint32_t which) noexcept override {
    if (auto* input = g_InputGenerator) {
      input->controllerAdded(which);
    } else {
      m_defferredControllers.emplace_back(which);
    }
  }

  void onControllerRemoved(uint32_t which) noexcept override {
    if (auto* input = g_InputGenerator) {
      input->controllerRemoved(which);
    }
  }

  void onAppExiting() noexcept override {
    m_imGuiConsole.Shutdown();
    if (g_mainMP1) {
      g_mainMP1->Shutdown();
    }
    g_mainMP1.reset();
    //    m_renderTex.reset();
    //    m_pipelineConv.reset();
    m_cvarManager.serialize();
    m_voiceEngine.reset();
    m_amuseAllocWrapper.reset();
    CDvdFile::Shutdown();
  }

  void onCharKeyDown(uint8_t code, aurora::ModifierKey mods, bool isRepeat) noexcept override {
    Log.report(logvisor::Info, FMT_STRING("DEBUG CHAR KEYS: '{}', isRepeat {}"), static_cast<char>(code), isRepeat);
    //    if (!ImGuiWindowCallback::m_keyboardCaptured && g_mainMP1) {
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->charKeyDown(code, mods, isRepeat);
      }
    }
    //    }
  }

  void onCharKeyUp(uint8_t code, aurora::ModifierKey mods) noexcept override {
    Log.report(logvisor::Info, FMT_STRING("DEBUG CHAR KEYS: '{}'"), static_cast<char>(code));
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->charKeyUp(code, mods);
      }
    }
  }

  void onSpecialKeyDown(aurora::SpecialKey key, aurora::ModifierKey mods, bool isRepeat) noexcept override {
    Log.report(logvisor::Info, FMT_STRING("DEBUG KEYS: SpecialKey {}, isRepeat {}"), key, isRepeat);
    /* TODO: Temporarily convert the aurora enum to boo's until we refactor everything */
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->specialKeyDown(key, mods, isRepeat);
      }
    }
    if (True(mods & (aurora::ModifierKey::LeftAlt | aurora::ModifierKey::RightAlt))) {
      if (key == aurora::SpecialKey::Enter) {
        m_fullscreenToggleRequested = true;
      } else if (key == aurora::SpecialKey::F4) {
        m_quitRequested = true;
      }
    }
  }

  void onSpecialKeyUp(aurora::SpecialKey key, aurora::ModifierKey mods) noexcept override {
    /* TODO: Temporarily convert the aurora enum to boo's until we refactor everything */
    if (g_mainMP1) {
      if (MP1::CGameArchitectureSupport* as = g_mainMP1->GetArchSupport()) {
        as->specialKeyUp(key, mods);
      }
    }
  }

  [[nodiscard]] std::string getGraphicsApi() const { return m_cvarCommons.getGraphicsApi(); }

  [[nodiscard]] uint32_t getSamples() const { return m_cvarCommons.getSamples(); }

  [[nodiscard]] uint32_t getAnisotropy() const { return m_cvarCommons.getAnisotropy(); }

  [[nodiscard]] bool getDeepColor() const { return m_cvarCommons.getDeepColor(); }

  [[nodiscard]] std::chrono::nanoseconds getTargetFrameTime() const {
    if (m_cvarCommons.getVariableFrameTime()) {
      return std::chrono::nanoseconds{0};
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{1}) / 60;
  }
};

} // namespace metaforce

static char CwdBuf[1024];
std::string ExeDir;

static void SetupBasics(bool logging) {
  auto result = zeus::validateCPU();
  if (!result.first) {
#if _WIN32 && !WINDOWS_STORE
    std::string msg =
        fmt::format(FMT_STRING("ERROR: This build of Metaforce requires the following CPU features:\n{}\n"),
                    metaforce::CPUFeatureString(result.second));
    MessageBoxW(nullptr, nowide::widen(msg).c_str(), L"CPU error", MB_OK | MB_ICONERROR);
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
  hecl::Runtime::FileStoreManager fileMgr{"sentry-native-metaforce"};
  std::string cacheDir{fileMgr.getStoreRoot()};
  logvisor::RegisterSentry("metaforce", METAFORCE_WC_DESCRIBE, cacheDir.c_str());
#endif
}

static bool IsClientLoggingEnabled(int argc, char** argv) {
  for (int i = 1; i < argc; ++i)
    if (!hecl::StrNCmp(argv[i], "-l", 2))
      return true;
  return false;
}

#if !WINDOWS_STORE
extern "C" void cxxbridge1$rust_vec$u8$set_len(rust::Vec<u8>* ptr, std::size_t len) noexcept;

int main(int argc, char** argv) {
  //TODO: This seems to fix a lot of weird issues with rounding
  // but breaks animations, need to research why this is the case
  // for now it's disabled
  //fesetround(FE_TOWARDZERO);
  if (argc > 1 && !hecl::StrCmp(argv[1], "--dlpackage")) {
    fmt::print(FMT_STRING("{}\n"), METAFORCE_DLPACKAGE);
    return 100;
  }

  SetupBasics(IsClientLoggingEnabled(argc, argv));
  hecl::Runtime::FileStoreManager fileMgr{"metaforce"};
  hecl::CVarManager cvarMgr{fileMgr};
  hecl::CVarCommons cvarCmns{cvarMgr};

  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i)
    args.emplace_back(argv[i]);
  cvarMgr.parseCommandLine(args);

  std::string logFile = cvarCmns.getLogFile();
  std::string logFilePath;
  if (!logFile.empty()) {
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buf[100];
    std::strftime(buf, 100, "%Y-%m-%d_%H-%M-%S", std::localtime(&time));
    logFilePath = fmt::format(FMT_STRING("{}/{}-{}"), fileMgr.getStoreRoot(), buf, logFile);
    logvisor::RegisterFileLogger(logFilePath.c_str());
  }

  if (char* cwd = hecl::Getcwd(CwdBuf, 1024)) {
    if (hecl::PathRelative(argv[0]))
      ExeDir = std::string(cwd) + '/';
    std::string Argv0(argv[0]);
    std::string::size_type lastIdx = Argv0.find_last_of("/\\");
    if (lastIdx != std::string::npos)
      ExeDir.insert(ExeDir.end(), Argv0.begin(), Argv0.begin() + lastIdx);
  }

  /* Handle -j argument */
  hecl::SetCpuCountOverride(argc, argv);

  auto app = std::make_unique<metaforce::Application>(fileMgr, cvarMgr, cvarCmns);
  auto data = aurora::Icon{};
  {
    auto icon = metaforce::GetIcon();
    data.data.reserve(icon.size);
    std::memcpy(data.data.data(), icon.data.get(), icon.size);
    // terrible hack: https://github.com/dtolnay/cxx/issues/990
    cxxbridge1$rust_vec$u8$set_len(&data.data, icon.size);
    data.width = icon.width;
    data.height = icon.height;
  }
  aurora::app_run(std::move(app), data);
  return 0;
}
#endif

#if WINDOWS_STORE
#include "boo/UWPViewProvider.hpp"
using namespace Windows::ApplicationModel::Core;

[Platform::MTAThread] int WINAPIV main(Platform::Array<Platform::String ^> ^ params) {
  SetupBasics(false);
  metaforce::Application appCb;
  auto viewProvider = ref new boo::ViewProvider(appCb, "metaforce", "Metaforce", "metaforce", params, false);
  CoreApplication::Run(viewProvider);
  return 0;
}

#elif _WIN32
#include <shellapi.h>
#include <nowide/args.hpp>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int) {
  int argc = 0;
  char** argv = nullptr;
  nowide::args _(argc, argv);
  const DWORD outType = GetFileType(GetStdHandle(STD_ERROR_HANDLE));
  if (IsClientLoggingEnabled(argc, argv) && outType == FILE_TYPE_UNKNOWN)
    logvisor::CreateWin32Console();
  return main(argc, argv);
}
#endif
