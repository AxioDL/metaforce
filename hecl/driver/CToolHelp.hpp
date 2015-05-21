#ifndef CTOOL_HELP
#define CTOOL_HELP

#include "CToolBase.hpp"
#include <stdio.h>
#include <stdexcept>

class CToolHelp final : public CToolBase
{
public:
    CToolHelp(const SToolPassInfo& info)
    : CToolBase(info)
    {
        if (m_info.args.empty())
            throw std::invalid_argument("help requires a tool name argument");
    }

    ~CToolHelp()
    {
    }

    static void Help()
    {
        printf("................................___________ \n"
               "...........................,.-'\"...........``~., \n"
               "........................,.-\".......................\"-., \n"
               "....................,/..................................\":, \n"
               "..................,?........................................, \n"
               "................/...........................................,}\n"
               "............../........................................,:`^`..}\n"
               "............./.......................................,:\"...../\n"
               "............?.....__..................................:`...../\n"
               ".........../__.(...\"~-,_...........................,:`....../\n"
               "........../(_....\"~,_....\"~,_.....................,:`...._/ \n"
               "..........{.._$;_....\"=,_.....\"-,_......,.-~-,},.~\";/....} \n"
               "...........((...*~_......\"=-._...\";,,./`........../\"..../ \n"
               "...,,,___.`~,......\"~.,....................`......}....../ \n"
               "............(....`=-,,...`.........................(...;_,,-\" \n"
               "............/.`~,......`-.................................../ \n"
               ".............`~.*-,.....................................|,./...,__ \n"
               ",,_..........}.>-._...................................|.......`=~-, \n"
               ".....`=~-,__......`,................................. \n"
               "...................`=~-,,.,........................... \n"
               ".........................`:,,..........................`\n"
               "...........................`=-,...............,%%`>--==`` \n"
               ".................................._.........._,-%%...` \n"
               "...................................,\n");
    }

    static void ToolHelp(const std::string& toolName)
    {
        if (toolName == "init")
            CToolInit::Help();
        else if (toolName == "add")
            CToolAdd::Help();
        else if (toolName == "remove" || toolName == "rm")
            CToolRemove::Help();
        else if (toolName == "group")
            CToolGroup::Help();
        else if (toolName == "cook")
            CToolCook::Help();
        else if (toolName == "clean")
            CToolClean::Help();
        else if (toolName == "package")
            CToolPackage::Help();
        else if (toolName == "help")
            CToolHelp::Help();
        else
            throw std::invalid_argument("unrecognized tool '" + toolName + "' - can't help");
    }

    std::string toolName() const {return "help";}

    int run()
    {
        ToolHelp(m_info.args[0]);
        return 0;
    }
};

#endif // CTOOL_HELP
