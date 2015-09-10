#if _WIN32
#define WIN_PAUSE 1
#include <objbase.h>
#endif

#include <clocale>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <regex>
#include <list>
#include "HECL/Database.hpp"
#include "../blender/BlenderConnection.hpp"
#include "LogVisor/LogVisor.hpp"

LogVisor::LogModule LogModule("HECLDriver");

#include "ToolBase.hpp"
#include "ToolInit.hpp"
#include "ToolSpec.hpp"
#include "ToolExtract.hpp"
#include "ToolAdd.hpp"
#include "ToolRemove.hpp"
#include "ToolGroup.hpp"
#include "ToolCook.hpp"
#include "ToolClean.hpp"
#include "ToolPackage.hpp"
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
static void printHelp(const HECL::SystemChar* pname)
{
    if (XTERM_COLOR)
        HECL::Printf(_S("" BOLD "HECL" NORMAL ""));
    else
        HECL::Printf(_S("HECL"));
#if HECL_GIT
    HECL::Printf(_S(" Commit " HECL_GIT_S " " HECL_BRANCH_S "\nUsage: %s init|add|remove|group|cook|clean|package|help\n"), pname);
#elif HECL_VER
    HECL::Printf(_S(" Version " HECL_VER_S "\nUsage: %s init|add|remove|group|cook|clean|package|help\n"), pname);
#else
    HECL::Printf(_S("\nUsage: %s init|add|remove|group|cook|clean|package|help\n"), pname);
#endif
}

/* Regex patterns */
static const HECL::SystemRegex regOPEN(_S("-o([^\"]*|\\S*)"), std::regex::ECMAScript|std::regex::optimize);
static const HECL::SystemRegex regVERBOSE(_S("-v(v*)"), std::regex::ECMAScript|std::regex::optimize);
static const HECL::SystemRegex regFORCE(_S("-f"), std::regex::ECMAScript|std::regex::optimize);

/* SIGINT will gracefully close blender connections and delete blends in progress */
static void SIGINTHandler(int sig)
{
    HECL::BlenderConnection::Shutdown();
    exit(1);
}

/* SIGWINCH should do nothing */
static void SIGWINCHHandler(int sig) {}

static LogVisor::LogModule AthenaLog("Athena");
static void AthenaExc(const Athena::error::Level& level, const char* file,
                      const char*, int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    AthenaLog.reportSource(LogVisor::Level(level), file, line, fmt, ap);
    va_end(ap);
}

#if HECL_UCS2
int wmain(int argc, const wchar_t** argv)
#else
int main(int argc, const char** argv)
#endif
{
#if _WIN32
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
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

    LogVisor::RegisterConsoleLogger();
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
    HECL::SystemChar cwdbuf[1024];
    if (HECL::Getcwd(cwdbuf, 1024))
    {
        info.cwd = cwdbuf;
        if (info.cwd.size() && info.cwd.back() != _S('/') && info.cwd.back() != _S('\\'))
#if _WIN32
            info.cwd += _S('\\');
#else
            info.cwd += _S('/');
#endif
    }

    /* Concatenate args */
    std::list<HECL::SystemString> args;
    for (int i=2 ; i<argc ; ++i)
        args.push_back(HECL::SystemString(argv[i]));

    if (!args.empty())
    {
        /* Extract output argument */
        for (std::list<HECL::SystemString>::const_iterator it = args.begin() ; it != args.end() ;)
        {
            const HECL::SystemString& arg = *it;
            HECL::SystemRegexMatch oMatch;
            if (std::regex_search(arg, oMatch, regOPEN))
            {
                const HECL::SystemString& token = oMatch[1].str();
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

        /* Count verbosity */
        for (std::list<HECL::SystemString>::const_iterator it = args.begin() ; it != args.end() ;)
        {
            const HECL::SystemString& arg = *it;
            HECL::SystemRegexMatch vMatch;
            if (std::regex_search(arg, vMatch, regVERBOSE))
            {
                ++info.verbosityLevel;
                info.verbosityLevel += vMatch[1].str().size();
                it = args.erase(it);
                continue;
            }
            ++it;
        }

        /* Check force argument */
        for (std::list<HECL::SystemString>::const_iterator it = args.begin() ; it != args.end() ;)
        {
            const HECL::SystemString& arg = *it;
            if (std::regex_search(arg, regFORCE))
            {
                info.force = true;
                it = args.erase(it);
                continue;
            }
            ++it;
        }

        /* Gather remaining args */
        for (const HECL::SystemString& arg : args)
            info.args.push_back(arg);
    }

    /* Attempt to find hecl project */
    std::unique_ptr<HECL::ProjectRootPath> rootPath = HECL::SearchForProject(info.cwd);
    std::unique_ptr<HECL::Database::Project> project;
    if (rootPath.get())
    {
        size_t ErrorRef = LogVisor::ErrorCount;
        HECL::Database::Project* newProj = new HECL::Database::Project(*rootPath);
        if (LogVisor::ErrorCount > ErrorRef)
        {
#if WIN_PAUSE
            system("PAUSE");
#endif
            delete newProj;
            return -1;
        }
        project.reset(newProj);
        info.project = newProj;
    }

    /* Construct selected tool */
    HECL::SystemString toolName(argv[1]);
    HECL::ToLower(toolName);
    std::unique_ptr<ToolBase> tool;

    size_t ErrorRef = LogVisor::ErrorCount;
    if (toolName == _S("init"))
        tool.reset(new ToolInit(info));
    else if (toolName == _S("spec"))
        tool.reset(new ToolSpec(info));
    else if (toolName == _S("extract"))
        tool.reset(new ToolExtract(info));
    else if (toolName == _S("add"))
        tool.reset(new ToolAdd(info));
    else if (toolName == _S("remove") || toolName == _S("rm"))
        tool.reset(new ToolRemove(info));
    else if (toolName == _S("group"))
        tool.reset(new ToolGroup(info));
    else if (toolName == _S("cook"))
        tool.reset(new ToolCook(info));
    else if (toolName == _S("clean"))
        tool.reset(new ToolClean(info));
    else if (toolName == _S("package") || toolName == _S("pack"))
        tool.reset(new ToolPackage(info));
    else if (toolName == _S("help"))
        tool.reset(new ToolHelp(info));
    else
    {
        FILE* fp = HECL::Fopen(argv[1], _S("rb"));
        if (!fp)
            LogModule.report(LogVisor::Error, _S("unrecognized tool '%s'"), toolName.c_str());
        else
        {
            /* Shortcut-case: implicit extract */
            fclose(fp);
            info.args.push_front(argv[1]);
            tool.reset(new ToolExtract(info));
        }
    }

    if (LogVisor::ErrorCount > ErrorRef)
    {
#if WIN_PAUSE
        system("PAUSE");
#endif
        return -1;
    }

    if (info.verbosityLevel)
        LogModule.report(LogVisor::Info, _S("Constructed tool '%s' %d\n"),
                         tool->toolName().c_str(), info.verbosityLevel);

    /* Run tool */
    ErrorRef = LogVisor::ErrorCount;
    int retval = tool->run();
    if (LogVisor::ErrorCount > ErrorRef)
    {
        HECL::BlenderConnection::Shutdown();
#if WIN_PAUSE
        system("PAUSE");
#endif
        return -1;
    }

    HECL::BlenderConnection::Shutdown();
#if WIN_PAUSE
    system("PAUSE");
#endif
    return retval;
}


