#include <string>
#include <string_view>
#include <numeric>
#include <iostream>

#include "ImGuiEngine.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/MP1/MP1.hpp"
#include "Runtime/ConsoleVariables/FileStoreManager.hpp"
#include "Runtime/ConsoleVariables/CVarManager.hpp"
#include "Runtime/CInfiniteLoopDetector.hpp"
#include "amuse/BooBackend.hpp"

#include "logvisor/logvisor.hpp"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <shellapi.h>
#endif

#include "../version.h"

// #include <fenv.h>
// #pragma STDC FENV_ACCESS ON

#include <aurora/event.h>
#include <aurora/main.h>
#include <dolphin/vi.h>
#include <SDL_messagebox.h>

using namespace std::literals;

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

struct Application {
private:
  int m_argc;
  char** m_argv;
  FileStoreManager& m_fileMgr;
  CVarManager& m_cvarManager;
  CVarCommons& m_cvarCommons;
  ImGuiConsole m_imGuiConsole;

  std::string m_deferredProject;
  bool m_projectInitialized = false;
  std::optional<amuse::BooBackendVoiceAllocator> m_amuseAllocWrapper;
  std::unique_ptr<boo::IAudioVoiceEngine> m_voiceEngine;

  Limiter m_limiter{};

  bool m_firstFrame = true;
  bool m_fullscreenToggleRequested = false;
  bool m_quitRequested = false;
  bool m_lAltHeld = false;
  using delta_clock = std::chrono::high_resolution_clock;
  delta_clock::time_point m_prevFrameTime;

  std::vector<u32> m_deferredControllers; // used to capture controllers added before CInputGenerator
                                          // is built, i.e during initialization

public:
  Application(int argc, char** argv, FileStoreManager& fileMgr, CVarManager& cvarMgr, CVarCommons& cvarCmns)
  : m_argc(argc)
  , m_argv(argv)
  , m_fileMgr(fileMgr)
  , m_cvarManager(cvarMgr)
  , m_cvarCommons(cvarCmns)
  , m_imGuiConsole(cvarMgr, cvarCmns) {}

  void onAppLaunched(const AuroraInfo& info) noexcept {
    initialize();

    VISetWindowTitle(
        fmt::format(FMT_STRING("Metaforce {} [{}]"), METAFORCE_WC_DESCRIBE, backend_name(info.backend)).c_str());

    m_voiceEngine = boo::NewAudioVoiceEngine("metaforce", "Metaforce");
    m_voiceEngine->setVolume(0.7f);
    m_amuseAllocWrapper.emplace(*m_voiceEngine);

#if TARGET_OS_IOS || TARGET_OS_TV
    m_deferredProject = std::string{m_fileMgr.getStoreRoot()} + "game.iso";
#else
    for (int i = 1; i < m_argc; ++i) {
      std::string arg = m_argv[i];
      if (m_deferredProject.empty() && !arg.starts_with('-') && !arg.starts_with('+'))
        m_deferredProject = arg;
      else if (arg == "--no-sound")
        m_voiceEngine->setVolume(0.f);
    }
#endif

    m_voiceEngine->startPump();
  }

  void initialize() {
    zeus::detectCPU();

    const zeus::CPUInfo& cpuInf = zeus::cpuFeatures();
    Log.report(logvisor::Info, FMT_STRING("CPU Name: {}"), cpuInf.cpuBrand);
    Log.report(logvisor::Info, FMT_STRING("CPU Vendor: {}"), cpuInf.cpuVendor);
    Log.report(logvisor::Info, FMT_STRING("CPU Features: {}"), CPUFeatureString(cpuInf));
  }

  void onSdlEvent(const SDL_Event& event) noexcept {
    switch (event.type) {
    case SDL_KEYDOWN:
      m_lAltHeld = event.key.keysym.sym == SDLK_LALT;
      // Toggle fullscreen on ALT+ENTER
      if (event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_ALT) != 0u && event.key.repeat == 0u) {
        m_cvarCommons.m_fullscreen->fromBoolean(!m_cvarCommons.m_fullscreen->toBoolean());
      }
      break;
    case SDL_KEYUP:
      if (m_lAltHeld && event.key.keysym.sym == SDLK_LALT) {
        m_imGuiConsole.ToggleVisible();
        m_lAltHeld = false;
      }
    }
  }

  bool onAppIdle(float realDt) noexcept {
#ifdef NDEBUG
    /* Ping the watchdog to let it know we're still alive */
    CInfiniteLoopDetector::UpdateWatchDog(std::chrono::system_clock::now());
#endif

    if (!m_projectInitialized && !m_deferredProject.empty()) {
      if (CDvdFile::Initialize(m_deferredProject)) {
        m_projectInitialized = true;
        m_cvarCommons.m_lastDiscPath->fromLiteral(m_deferredProject);
      } else {
        Log.report(logvisor::Error, FMT_STRING("Failed to open disc image '{}'"), m_deferredProject);
        m_imGuiConsole.m_errorString = fmt::format(FMT_STRING("Failed to open disc image '{}'"), m_deferredProject);
      }
      m_deferredProject.clear();
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
      VISetWindowFullscreen(m_cvarCommons.getFullscreen());
    }

    // Let CVarManager inform all CVar listeners of the CVar's state and clear all mdoified flags if necessary
    m_cvarManager.proc();

    if (!g_mainMP1 && m_projectInitialized) {
      g_mainMP1.emplace(nullptr, nullptr);
      auto result =
          g_mainMP1->Init(m_argc, m_argv, m_fileMgr, &m_cvarManager, m_voiceEngine.get(), *m_amuseAllocWrapper);
      if (!result.empty()) {
        Log.report(logvisor::Error, FMT_STRING("{}"), result);
        m_imGuiConsole.m_errorString = result;
        g_mainMP1.reset();
        CDvdFile::Shutdown();
        m_projectInitialized = false;
        m_cvarCommons.m_lastDiscPath->fromLiteral(""sv);
      }
    }

    float dt = 1 / 60.f;
    if (m_cvarCommons.m_variableDt->toBoolean()) {
      dt = std::min(realDt, 1 / 30.f);
    }

    m_imGuiConsole.PreUpdate();
    if (g_mainMP1) {
      if (m_voiceEngine) {
        m_voiceEngine->lockPump();
      }
      if (g_mainMP1->Proc(dt)) {
        return false;
      }
      if (m_voiceEngine) {
        m_voiceEngine->unlockPump();
      }
    }
    m_imGuiConsole.PostUpdate();
    if (!g_mainMP1 && m_imGuiConsole.m_gameDiscSelected) {
      std::optional<std::string> result;
      m_imGuiConsole.m_gameDiscSelected.swap(result);
      m_deferredProject = std::move(*result);
    }

    if (m_quitRequested || m_imGuiConsole.m_quitRequested || m_cvarManager.restartRequired()) {
      if (g_mainMP1) {
        g_mainMP1->Quit();
      } else {
        return false;
      }
    }
    return true;
  }

  void onAppDraw() noexcept {
    OPTICK_EVENT("Draw");
    if (g_Renderer != nullptr) {
      g_Renderer->BeginScene();
      if (g_mainMP1) {
        g_mainMP1->Draw();
      }
      g_Renderer->EndScene();
    }
  }

  void onAppPostDraw() noexcept {
    OPTICK_EVENT("PostDraw");
    if (m_voiceEngine) {
      m_voiceEngine->pumpAndMixVoices();
    }
#ifdef EMSCRIPTEN
    CDvdFile::DoWork();
#endif
    CGraphics::TickRenderTimings();
    ++logvisor::FrameIndex;
  }

  void onAppWindowResized(const AuroraWindowSize& size) noexcept {
    CGraphics::SetViewportResolution({static_cast<s32>(size.fb_width), static_cast<s32>(size.fb_height)});
  }

  void onAppDisplayScaleChanged(float scale) noexcept { ImGuiEngine_Initialize(scale); }

  void onControllerAdded(uint32_t which) noexcept { m_imGuiConsole.ControllerAdded(which); }

  void onControllerRemoved(uint32_t which) noexcept { m_imGuiConsole.ControllerRemoved(which); }

  void onAppExiting() noexcept {
    m_imGuiConsole.Shutdown();
    if (m_voiceEngine) {
      m_voiceEngine->unlockPump();
      m_voiceEngine->stopPump();
    }
    if (g_mainMP1) {
      g_mainMP1->Shutdown();
    }
    g_mainMP1.reset();
    m_cvarManager.serialize();
    m_amuseAllocWrapper.reset();
    m_voiceEngine.reset();
    CDvdFile::Shutdown();
  }

  void onImGuiInit(float scale) noexcept { ImGuiEngine_Initialize(scale); }

  void onImGuiAddTextures() noexcept { ImGuiEngine_AddTextures(); }

  [[nodiscard]] std::chrono::nanoseconds getTargetFrameTime() const {
    if (m_cvarCommons.getVariableFrameTime()) {
      return std::chrono::nanoseconds{0};
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{1}) / 60;
  }
};

} // namespace metaforce

static void SetupBasics() {
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

#if SENTRY_ENABLED
  std::string cacheDir{metaforce::FileStoreManager::instance()->getStoreRoot()};
  logvisor::RegisterSentry("metaforce", METAFORCE_WC_DESCRIBE, cacheDir.c_str());
#endif
}

static bool IsClientLoggingEnabled(int argc, char** argv) {
#ifdef EMSCRIPTEN
  return true;
#else
  for (int i = 1; i < argc; ++i) {
    if (!strncmp(argv[i], "-l", 2)) {
      return true;
    }
  }
  return false;
#endif
}

static std::unique_ptr<metaforce::Application> g_app;
static SDL_Window* g_window;
static bool g_paused;

static void aurora_log_callback(AuroraLogLevel level, const char* message, unsigned int len) {
  logvisor::Level severity = logvisor::Fatal;
  switch (level) {
  case LOG_DEBUG:
  case LOG_INFO:
    severity = logvisor::Info;
    break;
  case LOG_WARNING:
    severity = logvisor::Warning;
    break;
  case LOG_ERROR:
    severity = logvisor::Error;
    break;
  default:
    break;
  }
  if (level == LOG_FATAL) {
    auto msg = fmt::format(FMT_STRING("Metaforce encountered an internal error:\n\n{}"), message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Metaforce", msg.c_str(), g_window);
  }
  metaforce::Log.report(severity, FMT_STRING("{}"), message);
}

static void aurora_imgui_init_callback(const AuroraWindowSize* size) { g_app->onImGuiInit(size->scale); }

#if !WINDOWS_STORE
int main(int argc, char** argv) {
  // TODO: This seems to fix a lot of weird issues with rounding
  //  but breaks animations, need to research why this is the case
  //  for now it's disabled
  // fesetround(FE_TOWARDZERO);
  if (argc > 1 && !strcmp(argv[1], "--dlpackage")) {
    fmt::print(FMT_STRING("{}\n"), METAFORCE_DLPACKAGE);
    return 100;
  }

  metaforce::FileStoreManager fileMgr{"AxioDL", "metaforce"};
  SetupBasics();

  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  auto icon = metaforce::GetIcon();

  // FIXME: logvisor needs to copy this
  std::string logFilePath;

  bool restart = false;
  do {
    metaforce::CVarManager cvarMgr{fileMgr};
    metaforce::CVarCommons cvarCmns{cvarMgr};

    if (!restart) {
      cvarMgr.parseCommandLine(args);

      // TODO add clear loggers func to logvisor so we can recreate loggers on restart
      bool logging = IsClientLoggingEnabled(argc, argv);
#if _WIN32
      if (logging && GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_UNKNOWN) {
        logvisor::CreateWin32Console();
      }
#endif
      logvisor::RegisterStandardExceptions();
      if (logging) {
        logvisor::RegisterConsoleLogger();
      }

      std::string logFile = cvarCmns.getLogFile();
      if (!logFile.empty()) {
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buf[100];
        std::strftime(buf, 100, "%Y-%m-%d_%H-%M-%S", std::localtime(&time));
        logFilePath = fmt::format(FMT_STRING("{}/{}-{}"), fileMgr.getStoreRoot(), buf, logFile);
        logvisor::RegisterFileLogger(logFilePath.c_str());
      }
    }

    g_app = std::make_unique<metaforce::Application>(argc, argv, fileMgr, cvarMgr, cvarCmns);
    std::string configPath{fileMgr.getStoreRoot()};
    const AuroraConfig config{
        .appName = "Metaforce",
        .configPath = configPath.c_str(),
        .desiredBackend = metaforce::backend_from_string(cvarCmns.getGraphicsApi()),
        .msaa = cvarCmns.getSamples(),
        .maxTextureAnisotropy = static_cast<uint16_t>(cvarCmns.getAnisotropy()),
        .startFullscreen = cvarCmns.getFullscreen(),
        .iconRGBA8 = icon.data.get(),
        .iconWidth = icon.width,
        .iconHeight = icon.height,
        .logCallback = aurora_log_callback,
        .imGuiInitCallback = aurora_imgui_init_callback,
    };
    const auto info = aurora_initialize(argc, argv, &config);
    g_window = info.window;
    g_app->onImGuiAddTextures();
    g_app->onAppLaunched(info);
    g_app->onAppWindowResized(info.windowSize);
    while (!cvarMgr.restartRequired()) {
      const auto* event = aurora_update();
      bool exiting = false;
      while (event != nullptr && event->type != AURORA_NONE) {
        switch (event->type) {
        case AURORA_EXIT:
          exiting = true;
          break;
        case AURORA_SDL_EVENT:
          g_app->onSdlEvent(event->sdl);
          break;
        case AURORA_WINDOW_RESIZED:
          g_app->onAppWindowResized(event->windowSize);
          break;
        case AURORA_CONTROLLER_ADDED:
          g_app->onControllerAdded(event->controller);
          break;
        case AURORA_CONTROLLER_REMOVED:
          g_app->onControllerRemoved(event->controller);
          break;
        case AURORA_PAUSED:
          g_paused = true;
          break;
        case AURORA_UNPAUSED:
          g_paused = false;
          break;
        default:
          break;
        }
        if (exiting) {
          break;
        }
        ++event;
      }
      if (exiting) {
        break;
      }
      if (g_paused) {
        continue;
      }
      g_app->onAppIdle(1.f / 60.f /* TODO */);
      aurora_begin_frame();
      g_app->onAppDraw();
      aurora_end_frame();
      g_app->onAppPostDraw();
    }
    g_app->onAppExiting();
    aurora_shutdown();
    g_app.reset();

    restart = cvarMgr.restartRequired();
  } while (restart);
  return 0;
}
#endif
