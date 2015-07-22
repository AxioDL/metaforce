#ifndef CTOOL_HELP
#define CTOOL_HELP

#include "ToolBase.hpp"
#include <stdio.h>
#include <stdexcept>
#include <functional>

class ToolHelp final : public ToolBase
{


public:
    ToolHelp(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (m_info.args.empty())
            LogModule.report(LogVisor::FatalError, "help requires a tool name argument");
    }

    ~ToolHelp()
    {
    }



    static void Help(HelpOutput& help)
    {
        help.printBold(
            _S("................................___________ \n")
            _S("...........................,.-'\"...........``~., \n")
            _S("........................,.-\".......................\"-., \n")
            _S("....................,/..................................\":, \n")
            _S("..................,?........................................, \n")
            _S("................/...........................................,}\n")
            _S("............../........................................,:`^`..}\n")
            _S("............./.......................................,:\"...../\n")
            _S("............?.....__..................................:`...../\n")
            _S(".........../__.(...\"~-,_...........................,:`....../\n")
            _S("........../(_....\"~,_....\"~,_.....................,:`...._/ \n")
            _S("..........{.._$;_....\"=,_.....\"-,_......,.-~-,},.~\";/....} \n")
            _S("...........((...*~_......\"=-._...\";,,./`........../\"..../ \n")
            _S("...,,,___.`~,......\"~.,....................`......}....../ \n")
            _S("............(....`=-,,...`.........................(...;_,,-\" \n")
            _S("............/.`~,......`-.................................../ \n")
            _S(".............`~.*-,.....................................|,./...,__ \n")
            _S(",,_..........}.>-._...................................|.......`=~-, \n")
            _S(".....`=~-,__......`,................................. \n")
            _S("...................`=~-,,.,........................... \n")
            _S(".........................`:,,..........................`\n")
            _S("...........................`=-,...............,%%`>--==`` \n")
            _S(".................................._.........._,-%%...` \n")
            _S("...................................,\n"));
    }

    static void ShowHelp(const HECL::SystemString& toolName)
    {
        /* Select tool's help-text streamer */
        HelpOutput::HelpFunc helpFunc = NULL;
        if (toolName == _S("init"))
            helpFunc = ToolInit::Help;
        else if (toolName == _S("spec"))
            helpFunc = ToolSpec::Help;
        else if (toolName == _S("extract"))
            helpFunc = ToolExtract::Help;
        else if (toolName == _S("add"))
            helpFunc = ToolAdd::Help;
        else if (toolName == _S("remove") || toolName == _S("rm"))
            helpFunc = ToolRemove::Help;
        else if (toolName == _S("group"))
            helpFunc = ToolGroup::Help;
        else if (toolName == _S("cook"))
            helpFunc = ToolCook::Help;
        else if (toolName == _S("clean"))
            helpFunc = ToolClean::Help;
        else if (toolName == _S("package") || toolName == _S("pack"))
            helpFunc = ToolPackage::Help;
        else if (toolName == _S("help"))
            helpFunc = ToolHelp::Help;
        else
        {
            LogModule.report(LogVisor::FatalError, _S("unrecognized tool '%s' - can't help"), toolName.c_str());
            return;
        }

        HelpOutput ho(helpFunc);
        ho.go();
    }

    HECL::SystemString toolName() const {return _S("help");}

    int run()
    {
        ShowHelp(m_info.args[0]);
        return 0;
    }
};

#endif // CTOOL_HELP
