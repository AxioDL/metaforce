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
#include "logvisor/logvisor.hpp"

logvisor::Module LogModule("hecl::Driver");

#include "ToolBase.hpp"
#include "ToolInit.hpp"
#include "ToolSpec.hpp"
#include "ToolExtract.hpp"
#include "ToolCook.hpp"
#include "ToolPackage.hpp"
#include "ToolImage.hpp"
#include "ToolHelp.hpp"

/* Static reference to dataspec additions
 * (used by MSVC to definitively link DataSpecs) */
#include "../DataSpecRegistry.hpp"

bool XTERM_COLOR = false;


/*
#define HECL_GIT 1234567
#define HECL_GIT_S "1234567"
#define HECL_BRANCH master
#define HECL_BRANCH_S "master"
*/

/* Main usage message */
static void printHelp(const hecl::SystemChar* pname)
{
    if (XTERM_COLOR)
        hecl::Printf(_S("" BOLD "HECL" NORMAL ""));
    else
        hecl::Printf(_S("HECL"));
#if HECL_HAS_NOD
#  define TOOL_LIST "extract|init|cook|package|image|help"
#else
#  define TOOL_LIST "extract|init|cook|package|help"
#endif
#if HECL_GIT
    hecl::Printf(_S(" Commit " HECL_GIT_S " " HECL_BRANCH_S "\nUsage: %s " TOOL_LIST "\n"), pname);
#elif HECL_VER
    hecl::Printf(_S(" Version " HECL_VER_S "\nUsage: %s " TOOL_LIST "\n"), pname);
#else
    hecl::Printf(_S("\nUsage: %s " TOOL_LIST "\n"), pname);
#endif
}

/* Regex patterns */
static const hecl::SystemRegex regOPEN(_S("-o([^\"]*|\\S*)"), std::regex::ECMAScript|std::regex::optimize);

static ToolBase* ToolPtr = nullptr;

/* SIGINT will gracefully close blender connections and delete blends in progress */
static void SIGINTHandler(int sig)
{
    if (ToolPtr)
        ToolPtr->cancel();
    hecl::blender::Connection::Shutdown();
    logvisor::KillProcessTree();
    exit(1);
}

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file,
                      const char*, int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    AthenaLog.report(logvisor::Level(level), fmt, ap);
    va_end(ap);
}

static hecl::SystemChar cwdbuf[1024];
hecl::SystemString ExeDir;

#if _WIN32
int wmain(int argc, const wchar_t** argv)
#else
/* SIGWINCH should do nothing */
static void SIGWINCHHandler(int sig) {}
int main(int argc, const char** argv)
#endif
{
    if (argc > 1 && !hecl::StrCmp(argv[1], _S("--dlpackage")))
    {
        printf("%s\n", HECL_DLPACKAGE);
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

    /* Basic usage check */
    if (argc == 1)
    {
        printHelp(argv[0]);
#if WIN_PAUSE
        system("PAUSE");
#endif
        return 0;
    }
    else if (argc == 0)
    {
        printHelp(_S("hecl"));
#if WIN_PAUSE
        system("PAUSE");
#endif
        return 0;
    }

    /* Prepare DataSpecs */
    HECLRegisterDataSpecs();

    /* Assemble common tool pass info */
    ToolPassInfo info;
    info.pname = argv[0];
    if (hecl::Getcwd(cwdbuf, 1024))
    {
        info.cwd = cwdbuf;
        if (info.cwd.size() && info.cwd.back() != _S('/') && info.cwd.back() != _S('\\'))
#if _WIN32
            info.cwd += _S('\\');
#else
            info.cwd += _S('/');
#endif

        if (hecl::PathRelative(argv[0]))
            ExeDir = hecl::SystemString(cwdbuf) + _S('/');
        hecl::SystemString Argv0(argv[0]);
        hecl::SystemString::size_type lastIdx = Argv0.find_last_of(_S("/\\"));
        if (lastIdx != hecl::SystemString::npos)
            ExeDir.insert(ExeDir.end(), Argv0.begin(), Argv0.begin() + lastIdx);
    }

    /* Concatenate args */
    std::vector<hecl::SystemString> args;
    args.reserve(argc-2);
    for (int i=2 ; i<argc ; ++i)
        args.push_back(hecl::SystemString(argv[i]));

    if (!args.empty())
    {
        /* Extract output argument */
        for (auto it = args.cbegin() ; it != args.cend() ;)
        {
            const hecl::SystemString& arg = *it;
            hecl::SystemRegexMatch oMatch;
            if (std::regex_search(arg, oMatch, regOPEN))
            {
                const hecl::SystemString& token = oMatch[1].str();
                if (token.size())
                {
                    if (info.output.empty())
                        info.output = oMatch[1].str();
                    it = args.erase(it);
                }
                else
                {
                    it = args.erase(it);
                    if (it == args.end())
                        break;
                    if (info.output.empty())
                        info.output = *it;
                    it = args.erase(it);
                }
                continue;
            }
            ++it;
        }

        /* Iterate flags */
        for (auto it = args.cbegin() ; it != args.cend() ;)
        {
            const hecl::SystemString& arg = *it;
            if (arg.size() < 2 || arg[0] != _S('-') || arg[1] == _S('-'))
            {
                ++it;
                continue;
            }

            for (auto chit = arg.cbegin() + 1 ; chit != arg.cend() ; ++chit)
            {
                if (*chit == _S('v'))
                    ++info.verbosityLevel;
                else if (*chit == _S('f'))
                    info.force = true;
                else if (*chit == _S('y'))
                    info.yes = true;
                else if (*chit == _S('g'))
                    info.gui = true;
                else
                    info.flags.push_back(*chit);
            }

            it = args.erase(it);
        }

        /* Gather remaining args */
        info.args.reserve(args.size());
        for (const hecl::SystemString& arg : args)
            info.args.push_back(arg);
    }

    /* Attempt to find hecl project */
    hecl::ProjectRootPath rootPath = hecl::SearchForProject(info.cwd);
    std::unique_ptr<hecl::Database::Project> project;
    if (rootPath)
    {
        size_t ErrorRef = logvisor::ErrorCount;
        hecl::Database::Project* newProj = new hecl::Database::Project(rootPath);
        if (logvisor::ErrorCount > ErrorRef)
        {
#if WIN_PAUSE
            system("PAUSE");
#endif
            delete newProj;
            return 1;
        }
        project.reset(newProj);
        info.project = newProj;
    }

    /* Construct selected tool */
    hecl::SystemString toolName(argv[1]);
    hecl::ToLower(toolName);
    std::unique_ptr<ToolBase> tool;

    size_t ErrorRef = logvisor::ErrorCount;
    if (toolName == _S("init"))
        tool.reset(new ToolInit(info));
    else if (toolName == _S("spec"))
        tool.reset(new ToolSpec(info));
    else if (toolName == _S("extract"))
        tool.reset(new ToolExtract(info));
    else if (toolName == _S("cook"))
        tool.reset(new ToolCook(info));
    else if (toolName == _S("package") || toolName == _S("pack"))
        tool.reset(new ToolPackage(info));
#if HECL_HAS_NOD
    else if (toolName == _S("image"))
        tool.reset(new ToolImage(info));
#endif
    else if (toolName == _S("help"))
        tool.reset(new ToolHelp(info));
    else
    {
        FILE* fp = hecl::Fopen(argv[1], _S("rb"));
        if (!fp)
            LogModule.report(logvisor::Error, _S("unrecognized tool '%s'"), toolName.c_str());
        else
        {
            /* Shortcut-case: implicit extract */
            fclose(fp);
            info.args.insert(info.args.begin(), argv[1]);
            tool.reset(new ToolExtract(info));
        }
    }

    if (logvisor::ErrorCount > ErrorRef)
    {
#if WIN_PAUSE
        system("PAUSE");
#endif
        return 1;
    }

    if (info.verbosityLevel)
        LogModule.report(logvisor::Info, _S("Constructed tool '%s' %d\n"),
                         tool->toolName().c_str(), info.verbosityLevel);

    /* Run tool */
    ErrorRef = logvisor::ErrorCount;
    ToolPtr = tool.get();
    int retval = tool->run();
    ToolPtr = nullptr;
    if (logvisor::ErrorCount > ErrorRef)
    {
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


