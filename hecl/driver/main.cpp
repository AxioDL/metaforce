#if _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
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
#include "hecl/Database.hpp"
#include "hecl/Blender/BlenderConnection.hpp"
#include "logvisor/logvisor.hpp"

logvisor::Module LogModule("hecl::Driver");

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
static void printHelp(const hecl::SystemChar* pname)
{
    if (XTERM_COLOR)
        hecl::Printf(_S("" BOLD "HECL" NORMAL ""));
    else
        hecl::Printf(_S("HECL"));
#if HECL_GIT
    hecl::Printf(_S(" Commit " HECL_GIT_S " " HECL_BRANCH_S "\nUsage: %s init|add|remove|group|cook|clean|package|help\n"), pname);
#elif HECL_VER
    hecl::Printf(_S(" Version " HECL_VER_S "\nUsage: %s init|add|remove|group|cook|clean|package|help\n"), pname);
#else
    hecl::Printf(_S("\nUsage: %s init|add|remove|group|cook|clean|package|help\n"), pname);
#endif
}

/* Regex patterns */
static const hecl::SystemRegex regOPEN(_S("-o([^\"]*|\\S*)"), std::regex::ECMAScript|std::regex::optimize);

/* SIGINT will gracefully close blender connections and delete blends in progress */
static void SIGINTHandler(int sig)
{
    hecl::BlenderConnection::Shutdown();
    exit(1);
}

/* SIGWINCH should do nothing */
static void SIGWINCHHandler(int sig) {}

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file,
                      const char*, int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    AthenaLog.report(logvisor::Level(level), fmt, ap);
    va_end(ap);
}

#if _WIN32
int wmain(int argc, const wchar_t** argv)
#else
int main(int argc, const char** argv)
#endif
{
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
    hecl::SystemChar cwdbuf[1024];
    if (hecl::Getcwd(cwdbuf, 1024))
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
            return -1;
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
        return -1;
    }

    if (info.verbosityLevel)
        LogModule.report(logvisor::Info, _S("Constructed tool '%s' %d\n"),
                         tool->toolName().c_str(), info.verbosityLevel);

    /* Run tool */
    ErrorRef = logvisor::ErrorCount;
    int retval = tool->run();
    if (logvisor::ErrorCount > ErrorRef)
    {
        hecl::BlenderConnection::Shutdown();
#if WIN_PAUSE
        system("PAUSE");
#endif
        return -1;
    }

    hecl::BlenderConnection::Shutdown();
#if WIN_PAUSE
    system("PAUSE");
#endif
    return retval;
}


