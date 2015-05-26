#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex>
#include <stdexcept>
#include <list>
#include <HECLDatabase.hpp>

#include "CToolBase.hpp"
#include "CToolInit.hpp"
#include "CToolAdd.hpp"
#include "CToolRemove.hpp"
#include "CToolGroup.hpp"
#include "CToolCook.hpp"
#include "CToolClean.hpp"
#include "CToolPackage.hpp"
#include "CToolHelp.hpp"

bool XTERM_COLOR = false;

/* Main usage message */
static void printHelp(const char* pname)
{
    if (XTERM_COLOR)
        printf(BOLD "HECL" NORMAL);
    else
        printf("HECL");
#if HECL_GIT
    printf(" Commit " #HECL_GIT " (" #HECL_BRANCH ")\n"
           "Usage: %s init|add|remove|group|cook|clean|package|help\n", pname);
#elif HECL_VER
    printf(" Version " #HECL_VER "\n"
           "Usage: %s init|add|remove|group|cook|clean|package|help\n", pname);
#else
    printf("\n"
           "Usage: %s init|add|remove|group|cook|clean|package|help\n", pname);
#endif
}

/* Regex patterns */
static const std::regex regOPEN("-o([^\"]*|\\S*))", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regVERBOSE("-v(v*)", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regFORCE("-f", std::regex::ECMAScript|std::regex::optimize);

#include "../blender/CBlenderConnection.hpp"

int main(int argc, const char** argv)
{
    /* Xterm check */
    const char* term = getenv("TERM");
    if (!strncmp(term, "xterm", 5))
        XTERM_COLOR = true;

    //CBlenderConnection bconn(false);
    //return 0;

    /* Basic usage check */
    if (argc == 1)
    {
        printHelp(argv[0]);
        return 0;
    }
    else if (argc == 0)
    {
        printHelp("hecl");
        return 0;
    }

    /* Assemble common tool pass info */
    SToolPassInfo info;
    info.pname = argv[0];

    /* Concatenate args */
    std::list<std::string> args;
    for (int i=2 ; i<argc ; ++i)
        args.push_back(std::string(argv[i]));

    if (!args.empty())
    {
        /* Extract output argument */
        for (std::list<std::string>::const_iterator it = args.begin() ; it != args.end() ;)
        {
            const std::string& arg = *it;
            std::smatch oMatch;
            if (std::regex_search(arg, oMatch, regOPEN))
            {
                const std::string& token = oMatch[1].str();
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
        for (std::list<std::string>::const_iterator it = args.begin() ; it != args.end() ;)
        {
            const std::string& arg = *it;
            std::smatch vMatch;
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
        for (std::list<std::string>::const_iterator it = args.begin() ; it != args.end() ;)
        {
            const std::string& arg = *it;
            if (std::regex_search(arg, regFORCE))
            {
                info.force = true;
                it = args.erase(it);
                continue;
            }
            ++it;
        }

        /* Gather remaining args */
        for (const std::string& arg : args)
            info.args.push_back(arg);
    }

    /* Construct selected tool */
    std::string toolName(argv[1]);
    std::transform(toolName.begin(), toolName.end(), toolName.begin(), tolower);
    CToolBase* tool = NULL;
    try
    {
        if (toolName == "init")
            tool = new CToolInit(info);
        else if (toolName == "add")
            tool = new CToolAdd(info);
        else if (toolName == "remove" || toolName == "rm")
            tool = new CToolRemove(info);
        else if (toolName == "group")
            tool = new CToolGroup(info);
        else if (toolName == "cook")
            tool = new CToolCook(info);
        else if (toolName == "clean")
            tool = new CToolClean(info);
        else if (toolName == "package" || toolName == "pack")
            tool = new CToolPackage(info);
        else if (toolName == "help")
            tool = new CToolHelp(info);
        else
            throw std::invalid_argument("unrecognized tool '" + toolName + "'");
    }
    catch (std::exception& ex)
    {
        fprintf(stderr, "Unable to construct HECL tool '%s':\n%s\n", toolName.c_str(), ex.what());
        delete tool;
        return -1;
    }

    if (info.verbosityLevel)
        printf("Constructed tool '%s' %d\n", tool->toolName().c_str(), info.verbosityLevel);

    /* Run tool */
    int retval;
    try
    {
        retval = tool->run();
    }
    catch (std::exception& ex)
    {
        fprintf(stderr, "Error running HECL tool '%s':\n%s\n", toolName.c_str(), ex.what());
        delete tool;
        return -1;
    }

    delete tool;
    return retval;
}
