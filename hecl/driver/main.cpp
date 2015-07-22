#if _WIN32
#define WIN_PAUSE 1
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex>
#include <stdexcept>
#include <list>
#include "HECL/Database.hpp"
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
#if _WIN32
    HECL::Printf(_S("HECL"));
#else
    if (XTERM_COLOR)
        HECL::Printf(_S("" BOLD "HECL" NORMAL ""));
    else
        HECL::Printf(_S("HECL"));
#endif
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

#include "../blender/BlenderConnection.hpp"

#if HECL_UCS2
int wmain(int argc, const wchar_t** argv)
#else
int main(int argc, const char** argv)
#endif
{
    //dummy();
    /* Xterm check */
    const char* term = getenv("TERM");
    if (term && !strncmp(term, "xterm", 5))
        XTERM_COLOR = true;

    LogVisor::RegisterConsoleLogger();

    //CBlenderConnection bconn(false);
    //return 0;

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

    /* Assemble common tool pass info */
    ToolPassInfo info;
    info.pname = argv[0];
    HECL::SystemChar cwdbuf[1024];
    if (HECL::Getcwd(cwdbuf, 1024))
        info.cwd = cwdbuf;

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
        try
        {
            project.reset(new HECL::Database::Project(*rootPath));
            info.project = project.get();
        }
        catch (std::exception&)
        {
            LogModule.report(LogVisor::Error,
                             _S("Unable to open discovered project at '%s'"),
                             rootPath->getAbsolutePath().c_str());
#if WIN_PAUSE
            system("PAUSE");
#endif
            return -1;
        }
    }

    /* Construct selected tool */
    HECL::SystemString toolName(argv[1]);
    HECL::ToLower(toolName);
    std::unique_ptr<ToolBase> tool;
    try
    {
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
            LogModule.report(LogVisor::FatalError, _S("unrecognized tool '%s'"), toolName.c_str());
    }
    catch (std::exception& ex)
    {
#if HECL_UCS2
        LogModule.report(LogVisor::Error, _S("Unable to construct HECL tool '%s': %S"),
                         toolName.c_str(), ex.what());
#else
        LogModule.report(LogVisor::Error, _S("Unable to construct HECL tool '%s': %s"),
                         toolName.c_str(), ex.what());
#endif
#if WIN_PAUSE
        system("PAUSE");
#endif
        return -1;
    }

    if (info.verbosityLevel)
        LogModule.report(LogVisor::Info, _S("Constructed tool '%s' %d\n"),
                         tool->toolName().c_str(), info.verbosityLevel);

    /* Run tool */
    int retval;
    try
    {
        retval = tool->run();
    }
    catch (std::exception& ex)
    {
#if HECL_UCS2
        LogModule.report(LogVisor::Error, _S("Error running HECL tool '%s': %S"),
                         toolName.c_str(), ex.what());
#else
        LogModule.report(LogVisor::Error, _S("Error running HECL tool '%s': %s"),
                         toolName.c_str(), ex.what());
#endif
#if WIN_PAUSE
        system("PAUSE");
#endif
        return -1;
    }

#if WIN_PAUSE
    system("PAUSE");
#endif
    return retval;
}


