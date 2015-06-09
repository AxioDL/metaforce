#ifndef CTOOL_HELP
#define CTOOL_HELP

#include "CToolBase.hpp"
#include <stdio.h>
#include <stdexcept>
#include <functional>

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



    static void Help(CHelpOutput& help)
    {
        help.printBold("................................___________ \n"
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
        /* Select tool's help-text streamer */
        CHelpOutput::THelpFunc helpFunc = NULL;
        if (toolName == "init")
            helpFunc = CToolInit::Help;
        else if (toolName == "spec")
            helpFunc = CToolSpec::Help;
        else if (toolName == "add")
            helpFunc = CToolAdd::Help;
        else if (toolName == "remove" || toolName == "rm")
            helpFunc = CToolRemove::Help;
        else if (toolName == "group")
            helpFunc = CToolGroup::Help;
        else if (toolName == "cook")
            helpFunc = CToolCook::Help;
        else if (toolName == "clean")
            helpFunc = CToolClean::Help;
        else if (toolName == "package" || toolName == "pack")
            helpFunc = CToolPackage::Help;
        else if (toolName == "help")
            helpFunc = CToolHelp::Help;
        else
        {
            throw std::invalid_argument("unrecognized tool '" + toolName + "' - can't help");
            return;
        }

        CHelpOutput ho(helpFunc);
        ho.go();
    }

    std::string toolName() const {return "help";}

    int run()
    {
        ToolHelp(m_info.args[0]);
        return 0;
    }
};

#endif // CTOOL_HELP
