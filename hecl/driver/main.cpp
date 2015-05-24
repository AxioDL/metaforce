#include <stdio.h>
#include <string.h>
#include <regex>
#include <stdexcept>
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

/* Main usage message */
static void printHelp(const char* pname)
{
#if HECL_GIT
    printf("HECL Commit " #HECL_GIT " (" #HECL_BRANCH ")\n"
           "Usage: %s init|add|remove|group|cook|clean|package|help\n", pname);
#elif HECL_VER
    printf("HECL Version " #HECL_VER "\n"
           "Usage: %s init|add|remove|group|cook|clean|package|help\n", pname);
#else
    printf("HECL\n"
           "Usage: %s init|add|remove|group|cook|clean|package|help\n", pname);
#endif
}

/* Regex patterns */
static const std::regex regOPEN("-o\\s*(\\S+)", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regVERBOSE("-v(v*)", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regFORCE("-f", std::regex::ECMAScript|std::regex::optimize);
static const std::regex regNOWS("\\S+", std::regex::ECMAScript|std::regex::optimize);

/* Iterates string segments around matched arguments and
 * filters args string accordingly */
static void whiddleArgs(std::string& args, const std::regex& regex)
{
    std::string remArgs;
    for (std::sregex_token_iterator it(args.begin(), args.end(), regex, -1);
         it != std::sregex_token_iterator() ; ++it)
    {
        const std::string& str = *it;
        remArgs += str;
    }
    args = remArgs;
}

#include "../blender/CBlenderConnection.hpp"

int main(int argc, const char** argv)
{
    CBlenderConnection bconn(false);
    return 0;

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
    std::string args;
    for (int i=2 ; i<argc ; ++i)
        args += std::string(argv[i]) + " ";

    if (!args.empty())
    {
        /* Extract output argument */
        std::sregex_token_iterator openIt(args.begin(), args.end(), regOPEN, 1);
        if (openIt != std::sregex_token_iterator())
        {
            if (info.output.empty())
                info.output = *openIt;
            whiddleArgs(args, regOPEN);
        }

        /* Count verbosity */
        for (std::sregex_token_iterator it(args.begin(), args.end(), regVERBOSE, 1);
             it != std::sregex_token_iterator() ; ++it)
        {
            const std::string& str = *it;
            ++info.verbosityLevel;
            info.verbosityLevel += str.length();
        }
        whiddleArgs(args, regVERBOSE);

        /* Check force argument */
        if (std::regex_search(args, regFORCE))
        {
            info.force = true;
            whiddleArgs(args, regFORCE);
        }

        /* Gather remaining args */
        for (std::sregex_token_iterator it(args.begin(), args.end(), regNOWS);
             it != std::sregex_token_iterator() ; ++it)
        {
            const std::string& str = *it;
            info.args.push_back(str);
        }
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
        else if (toolName == "package")
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
