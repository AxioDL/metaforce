#pragma once

#include "ToolBase.hpp"
#include <cstdio>
#include <functional>

class ToolHelp final : public ToolBase {

public:
  ToolHelp(const ToolPassInfo& info) : ToolBase(info) {
    if (m_info.args.empty()) {
      LogModule.report(logvisor::Error, "help requires a tool name argument");
      return;
    }
    m_good = true;
  }

  ~ToolHelp() {}

  static void Help(HelpOutput& help) {
    help.printBold(
        _SYS_STR("                                ___________ \n") _SYS_STR(
            "                           ,.-'\"...........``~., \n") _SYS_STR("                        "
                                                                             ",.-\".......................\"-., \n")
            _SYS_STR("                    ,/..................................\":, \n") _SYS_STR(
                "                 .,?........................................, \n")
                _SYS_STR("                /...........................................,}\n") _SYS_STR(
                    "             ./........................................,:`^`..}\n")
                    _SYS_STR("            ./.......................................,:\"...../\n") _SYS_STR(
                        "            ?.....__..................................:`...../\n")
                        _SYS_STR("           /__.(...\"~-,_...........................,:`....../\n") _SYS_STR(
                            "          /(_....\"~,_....\"~,_.....................,:`...._/ \n")
                            _SYS_STR("          {.._$;_....\"=,_.....\"-,_......,.-~-,},.~\";/....} \n") _SYS_STR(
                                "           ((...*~_......\"=-._...\";,,./`........../\"..../ \n")
                                _SYS_STR("   ,,,___.`~,......\"~.,....................`......}....../ \n") _SYS_STR(
                                    "............(....`=-,,...`.........................(...;_,,-\" \n")
                                    _SYS_STR("............/.`~,......`-.................................../ \n")
                                        _SYS_STR(".............`~.*-,.....................................|,./...,__ "
                                                 "\n") _SYS_STR(",,_..........}.>-._..................................."
                                                                "|.......`=~-, \n") _SYS_STR(
                                            ".....`=~-,__......`,................................. \n")
                                            _SYS_STR("...................`=~-,,.,........................... \n")
                                                _SYS_STR(".........................`:,,..........................`\n")
                                                    _SYS_STR(
                                                        "...........................`=-,...............,%%`>--==`` \n")
                                                        _SYS_STR(
                                                            ".................................._.........._,-%%...` \n")
                                                            _SYS_STR("...................................,\n"));
  }

  static void ShowHelp(const hecl::SystemString& toolName) {
    /* Select tool's help-text streamer */
    HelpOutput::HelpFunc helpFunc = NULL;
    if (toolName == _SYS_STR("init"))
      helpFunc = ToolInit::Help;
    else if (toolName == _SYS_STR("spec"))
      helpFunc = ToolSpec::Help;
    else if (toolName == _SYS_STR("extract"))
      helpFunc = ToolExtract::Help;
    else if (toolName == _SYS_STR("cook"))
      helpFunc = ToolCook::Help;
    else if (toolName == _SYS_STR("package") || toolName == _SYS_STR("pack"))
      helpFunc = ToolPackage::Help;
    else if (toolName == _SYS_STR("help"))
      helpFunc = ToolHelp::Help;
    else {
      LogModule.report(logvisor::Error, _SYS_STR("unrecognized tool '%s' - can't help"), toolName.c_str());
      return;
    }

    HelpOutput ho(helpFunc);
    ho.go();
  }

  hecl::SystemString toolName() const { return _SYS_STR("help"); }

  int run() {
    ShowHelp(m_info.args.front());
    return 0;
  }
};
