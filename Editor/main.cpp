#include "logvisor/logvisor.hpp"
#include "boo/boo.hpp"
#include "specter/specter.hpp"
#include "hecl/CVarManager.hpp"
#include "Runtime/CBasics.hpp"
#include "ViewManager.hpp"
#include "hecl/hecl.hpp"
#include "hecl/CVarCommons.hpp"
#include "hecl/Console.hpp"

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file, const char*, int line, fmt::string_view fmt,
                      fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

namespace urde {
static logvisor::Module Log{"URDE"};

static hecl::SystemString CPUFeatureString(const zeus::CPUInfo& cpuInf) {
  hecl::SystemString features;
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
  return features;
}

struct Application : boo::IApplicationCallback {
  hecl::Runtime::FileStoreManager m_fileMgr;
  hecl::CVarManager m_cvarManager;
  hecl::CVarCommons m_cvarCommons;
  std::unique_ptr<ViewManager> m_viewManager;

  std::atomic_bool m_running = {true};

  Application()
  : m_fileMgr(_SYS_STR("urde"))
  , m_cvarManager(m_fileMgr)
  , m_cvarCommons(m_cvarManager)
  , m_viewManager(std::make_unique<ViewManager>(m_fileMgr, m_cvarManager)) {}

  virtual ~Application() = default;

  int appMain(boo::IApplication* app) override {
    initialize(app);
    m_viewManager->init(app);
    while (m_running.load()) {
      if (!m_viewManager->proc())
        break;
    }
    m_viewManager->stop();
    m_viewManager->projectManager().saveProject();
    m_cvarManager.serialize();
    m_viewManager.reset();
    return 0;
  }
  void appQuitting(boo::IApplication*) override { m_running.store(false); }
  void appFilesOpen(boo::IApplication*, const std::vector<boo::SystemString>& paths) override {
    for (const auto& path : paths) {
      hecl::ProjectRootPath projPath = hecl::SearchForProject(path);
      if (projPath) {
        m_viewManager->deferOpenProject(path);
        break;
      }
    }
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

    m_cvarManager.parseCommandLine(app->getArgs());

    const zeus::CPUInfo& cpuInf = zeus::cpuFeatures();
    Log.report(logvisor::Info, FMT_STRING("CPU Name: {}"), cpuInf.cpuBrand);
    Log.report(logvisor::Info, FMT_STRING("CPU Vendor: {}"), cpuInf.cpuVendor);
    Log.report(logvisor::Info, FMT_STRING(_SYS_STR("CPU Features: {}")), CPUFeatureString(cpuInf));
  }

  std::string getGraphicsApi() const { return m_cvarCommons.getGraphicsApi(); }

  uint32_t getSamples() const { return m_cvarCommons.getSamples(); }

  uint32_t getAnisotropy() const { return m_cvarCommons.getAnisotropy(); }

  bool getDeepColor() const { return m_cvarCommons.getDeepColor(); }
};

} // namespace urde

static hecl::SystemChar CwdBuf[1024];
hecl::SystemString ExeDir;

static void SetupBasics(bool logging) {
  auto result = zeus::validateCPU();
  if (!result.first) {
#if _WIN32 && !WINDOWS_STORE
    std::wstring msg = fmt::format(FMT_STRING(L"ERROR: This build of URDE requires the following CPU features:\n{}\n"),
                                   urde::CPUFeatureString(result.second));
    MessageBoxW(nullptr, msg.c_str(), L"CPU error", MB_OK | MB_ICONERROR);
#else
    fmt::print(stderr, FMT_STRING("ERROR: This build of URDE requires the following CPU features:\n{}\n"),
               urde::CPUFeatureString(result.second));
#endif
    exit(1);
  }

  logvisor::RegisterStandardExceptions();
  if (logging)
    logvisor::RegisterConsoleLogger();
  atSetExceptionHandler(AthenaExc);
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
    fmt::print(FMT_STRING("{}\n"), URDE_DLPACKAGE);
    return 100;
  }

  SetupBasics(IsClientLoggingEnabled(argc, argv));

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

  urde::Application appCb;
  int ret = boo::ApplicationRun(boo::IApplication::EPlatformType::Auto, appCb, _SYS_STR("urde"), _SYS_STR("URDE"), argc,
                                argv, appCb.getGraphicsApi(), appCb.getSamples(), appCb.getAnisotropy(),
                                appCb.getDeepColor(), false);
  // printf("IM DYING!!\n");
  return ret;
}
#endif

#if WINDOWS_STORE
#include "boo/UWPViewProvider.hpp"
using namespace Windows::ApplicationModel::Core;

[Platform::MTAThread] int WINAPIV main(Platform::Array<Platform::String ^> ^ params) {
  SetupBasics(false);
  urde::Application appCb;
  auto viewProvider =
      ref new boo::ViewProvider(appCb, _SYS_STR("urde"), _SYS_STR("URDE"), _SYS_STR("urde"), params, false);
  CoreApplication::Run(viewProvider);
  return 0;
}

#elif _WIN32
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

  const DWORD outType = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
  if (IsClientLoggingEnabled(argc + 1, booArgv) && outType == FILE_TYPE_UNKNOWN)
    logvisor::CreateWin32Console();
  return wmain(argc + 1, booArgv);
}
#endif
