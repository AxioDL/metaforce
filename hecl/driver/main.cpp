#if _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN_PAUSE 0
#include <objbase.h>
#endif

#include <clocale>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <signal.h>
#include <regex>
#include <list>
#include "hecl/Database.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/Runtime.hpp"
#include "logvisor/logvisor.hpp"
#include "../version.h"

logvisor::Module LogModule("hecl::Driver");

#include "ToolBase.hpp"
#include "ToolInit.hpp"
#include "ToolSpec.hpp"
#include "ToolExtract.hpp"
#include "ToolCook.hpp"
#include "ToolPackage.hpp"
#include "ToolImage.hpp"
#include "ToolInstallAddon.hpp"
#include "ToolHelp.hpp"

/* Static reference to dataspec additions
 * (used by MSVC to definitively link DataSpecs) */
#include "DataSpecRegistry.hpp"

bool XTERM_COLOR = false;

/*
#define HECL_GIT 1234567
#define HECL_GIT_S "1234567"
#define HECL_BRANCH master
#define HECL_BRANCH_S "master"
*/

/* Main usage message */
static void printHelp(const hecl::SystemChar* pname) {
  if (XTERM_COLOR)
    fmt::print(FMT_STRING(_SYS_STR("" BOLD "HECL" NORMAL "")));
  else
    fmt::print(FMT_STRING(_SYS_STR("HECL")));
#if HECL_HAS_NOD
#define TOOL_LIST "extract|init|cook|package|image|installaddon|help"
#else
#define TOOL_LIST "extract|init|cook|package|installaddon|help"
#endif
#if HECL_GIT
  fmt::print(FMT_STRING(_SYS_STR(" Commit " HECL_GIT_S " " HECL_BRANCH_S "\nUsage: {} " TOOL_LIST "\n")), pname);
#elif HECL_VER
  fmt::print(FMT_STRING(_SYS_STR(" Version " HECL_VER_S "\nUsage: {} " TOOL_LIST "\n")), pname);
#else
  fmt::print(FMT_STRING(_SYS_STR("\nUsage: {} " TOOL_LIST "\n")), pname);
#endif
}

/* Regex patterns */
static const hecl::SystemRegex regOPEN(_SYS_STR("-o([^\"]*|\\S*)"), std::regex::ECMAScript | std::regex::optimize);

static ToolBase* ToolPtr = nullptr;

/* SIGINT will gracefully close blender connections and delete blends in progress */
static void SIGINTHandler(int sig) {
  if (ToolPtr)
    ToolPtr->cancel();
  hecl::blender::Connection::Shutdown();
  logvisor::KillProcessTree();
  exit(1);
}

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file, const char*, int line, fmt::string_view fmt,
                      fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

hecl::SystemString ExeDir;

#if _WIN32
static ToolPassInfo CreateInfo(int argc, const wchar_t** argv) {
#else
static ToolPassInfo CreateInfo(int argc, const char** argv) {
#endif
  hecl::SystemChar cwdbuf[1024];

  ToolPassInfo info;
  info.pname = argv[0];

  if (hecl::Getcwd(cwdbuf, static_cast<int>(std::size(cwdbuf)))) {
    info.cwd = cwdbuf;
    if (info.cwd.size() && info.cwd.back() != _SYS_STR('/') && info.cwd.back() != _SYS_STR('\\')) {
#if _WIN32
      info.cwd += _SYS_STR('\\');
#else
      info.cwd += _SYS_STR('/');
#endif
    }

    if (hecl::PathRelative(argv[0])) {
      ExeDir = hecl::SystemString(cwdbuf) + _SYS_STR('/');
    }
    hecl::SystemString Argv0(argv[0]);
    hecl::SystemString::size_type lastIdx = Argv0.find_last_of(_SYS_STR("/\\"));
    if (lastIdx != hecl::SystemString::npos) {
      ExeDir.insert(ExeDir.end(), Argv0.begin(), Argv0.begin() + lastIdx);
    }
  }

  /* Concatenate args */
  std::vector<hecl::SystemString> args;
  args.reserve(argc - 2);
  for (int i = 2; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  if (!args.empty()) {
    /* Extract output argument */
    for (auto it = args.cbegin(); it != args.cend();) {
      const hecl::SystemString& arg = *it;
      hecl::SystemRegexMatch oMatch;

      if (std::regex_search(arg, oMatch, regOPEN)) {
        const hecl::SystemString& token = oMatch[1].str();

        if (token.size()) {
          if (info.output.empty()) {
            info.output = oMatch[1].str();
          }

          it = args.erase(it);
        } else {
          it = args.erase(it);

          if (it == args.end()) {
            break;
          }

          if (info.output.empty()) {
            info.output = *it;
          }

          it = args.erase(it);
        }

        continue;
      }

      ++it;
    }

    /* Iterate flags */
    bool threadArg = false;
    for (auto it = args.cbegin(); it != args.cend();) {
      const hecl::SystemString& arg = *it;
      if (threadArg) {
        threadArg = false;
        hecl::CpuCountOverride = int(hecl::StrToUl(arg.c_str(), nullptr, 0));
        it = args.erase(it);
        continue;
      }
      if (arg.size() < 2 || arg[0] != _SYS_STR('-') || arg[1] == _SYS_STR('-')) {
        ++it;
        continue;
      }

      for (auto chit = arg.cbegin() + 1; chit != arg.cend(); ++chit) {
        if (*chit == _SYS_STR('v'))
          ++info.verbosityLevel;
        else if (*chit == _SYS_STR('f'))
          info.force = true;
        else if (*chit == _SYS_STR('y'))
          info.yes = true;
        else if (*chit == _SYS_STR('g'))
          info.gui = true;
        else if (*chit == _SYS_STR('j')) {
          ++chit;
          if (*chit)
            hecl::CpuCountOverride = int(hecl::StrToUl(&*chit, nullptr, 0));
          else
            threadArg = true;
          break;
        } else
          info.flags.push_back(*chit);
      }

      it = args.erase(it);
    }

    /* Gather remaining args */
    info.args.reserve(args.size());
    for (const hecl::SystemString& arg : args)
      info.args.push_back(arg);
  }

  return info;
}

static std::unique_ptr<hecl::Database::Project> FindProject(hecl::SystemStringView cwd) {
  const hecl::ProjectRootPath rootPath = hecl::SearchForProject(cwd);
  if (!rootPath) {
    return nullptr;
  }

  const size_t ErrorRef = logvisor::ErrorCount;
  auto newProj = std::make_unique<hecl::Database::Project>(rootPath);
  if (logvisor::ErrorCount > ErrorRef) {
#if WIN_PAUSE
    system("PAUSE");
#endif
    return nullptr;
  }

  return newProj;
}

static std::unique_ptr<ToolBase> MakeSelectedTool(hecl::SystemString toolName, ToolPassInfo& info) {
  hecl::SystemString toolNameLower = toolName;
  hecl::ToLower(toolNameLower);

  if (toolNameLower == _SYS_STR("init")) {
    return std::make_unique<ToolInit>(info);
  }

  if (toolNameLower == _SYS_STR("spec")) {
    return std::make_unique<ToolSpec>(info);
  }

  if (toolNameLower == _SYS_STR("extract")) {
    return std::make_unique<ToolExtract>(info);
  }

  if (toolNameLower == _SYS_STR("cook")) {
    return std::make_unique<ToolCook>(info);
  }

  if (toolNameLower == _SYS_STR("package") || toolNameLower == _SYS_STR("pack")) {
    return std::make_unique<ToolPackage>(info);
  }

#if HECL_HAS_NOD
  if (toolNameLower == _SYS_STR("image")) {
    return std::make_unique<ToolImage>(info);
  }
#endif

  if (toolNameLower == _SYS_STR("installaddon")) {
    return std::make_unique<ToolInstallAddon>(info);
  }

  if (toolNameLower == _SYS_STR("help")) {
    return std::make_unique<ToolHelp>(info);
  }

  auto fp = hecl::FopenUnique(toolName.c_str(), _SYS_STR("rb"));
  if (fp == nullptr) {
    LogModule.report(logvisor::Error, FMT_STRING(_SYS_STR("unrecognized tool '{}'")), toolNameLower);
    return nullptr;
  }
  fp.reset();

  /* Shortcut-case: implicit extract */
  info.args.insert(info.args.begin(), std::move(toolName));
  return std::make_unique<ToolExtract>(info);
}

#if _WIN32
int wmain(int argc, const wchar_t** argv)
#else
/* SIGWINCH should do nothing */
static void SIGWINCHHandler(int sig) {}
int main(int argc, const char** argv)
#endif
{
  if (argc > 1 && !hecl::StrCmp(argv[1], _SYS_STR("--dlpackage"))) {
    fmt::print(FMT_STRING("{}\n"), URDE_DLPACKAGE);
    return 100;
  }

#if _WIN32
  CoInitializeEx(nullptr, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
#else
  std::setlocale(LC_ALL, "en-US.UTF-8");
#endif

  /* Xterm check */
#if _WIN32
  const char* conemuANSI = getenv("ConEmuANSI");
  if (conemuANSI && !strcmp(conemuANSI, "ON"))
    XTERM_COLOR = true;
#else
  const char* term = getenv("TERM");
  if (term && !strncmp(term, "xterm", 5))
    XTERM_COLOR = true;
  signal(SIGWINCH, SIGWINCHHandler);
#endif
  signal(SIGINT, SIGINTHandler);

  logvisor::RegisterStandardExceptions();
  logvisor::RegisterConsoleLogger();
  atSetExceptionHandler(AthenaExc);

#if SENTRY_ENABLED
  hecl::Runtime::FileStoreManager fileMgr{_SYS_STR("sentry-native-hecl")};
  hecl::SystemUTF8Conv cacheDir{fileMgr.getStoreRoot()};
  logvisor::RegisterSentry("hecl", URDE_WC_DESCRIBE, cacheDir.c_str());
#endif

  /* Basic usage check */
  if (argc == 1) {
    printHelp(argv[0]);
#if WIN_PAUSE
    system("PAUSE");
#endif
    return 0;
  } else if (argc == 0) {
    printHelp(_SYS_STR("hecl"));
#if WIN_PAUSE
    system("PAUSE");
#endif
    return 0;
  }

  /* Prepare DataSpecs */
  HECLRegisterDataSpecs();

  /* Assemble common tool pass info */
  ToolPassInfo info = CreateInfo(argc, argv);

  /* Attempt to find hecl project */
  auto project = FindProject(info.cwd);
  if (project != nullptr) {
    info.project = project.get();
  }

  /* Construct selected tool */
  const size_t MakeToolErrorRef = logvisor::ErrorCount;
  auto tool = MakeSelectedTool(argv[1], info);
  if (logvisor::ErrorCount > MakeToolErrorRef) {
#if WIN_PAUSE
    system("PAUSE");
#endif
    return 1;
  }
  if (info.verbosityLevel) {
    LogModule.report(logvisor::Info, FMT_STRING(_SYS_STR("Constructed tool '{}' {}\n")), tool->toolName(),
                     info.verbosityLevel);
  }

  /* Run tool */
  const size_t RunToolErrorRef = logvisor::ErrorCount;
  ToolPtr = tool.get();
  const int retval = tool->run();
  ToolPtr = nullptr;
  if (logvisor::ErrorCount > RunToolErrorRef) {
    hecl::blender::Connection::Shutdown();
#if WIN_PAUSE
    system("PAUSE");
#endif
    return 1;
  }

  hecl::blender::Connection::Shutdown();
#if WIN_PAUSE
  system("PAUSE");
#endif
  return retval;
}
