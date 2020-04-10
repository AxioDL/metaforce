#pragma once

#include "ToolBase.hpp"
#include <cstdio>
#include <functional>

class ToolHelp final : public ToolBase {

public:
  explicit ToolHelp(const ToolPassInfo& info) : ToolBase(info) {
    if (m_info.args.empty()) {
      LogModule.report(logvisor::Error, fmt("help requires a tool name argument"));
      return;
    }
    m_good = true;
  }

  ~ToolHelp() override = default;

  static void Help(HelpOutput& help) {
    /* clang-format off */
    help.printBold(
        _SYS_STR("                                ___________ \n")
        _SYS_STR("                           ,.-'\"...........``~., \n")
        _SYS_STR("                        ,.-\".......................\"-., \n")
        _SYS_STR("                    ,/..................................\":, \n")
        _SYS_STR("                 .,?........................................, \n")
        _SYS_STR("                /...........................................,}\n")
        _SYS_STR("             ./........................................,:`^`..}\n")
        _SYS_STR("            ./.......................................,:\"...../\n")
        _SYS_STR("            ?.....__..................................:`...../\n")
        _SYS_STR("           /__.(...\"~-,_...........................,:`....../\n")
        _SYS_STR("          /(_....\"~,_....\"~,_.....................,:`...._/ \n")
        _SYS_STR("          {.._$;_....\"=,_.....\"-,_......,.-~-,},.~\";/....} \n")
        _SYS_STR("           ((...*~_......\"=-._...\";,,./`........../\"..../ \n")
        _SYS_STR("   ,,,___.`~,......\"~.,....................`......}....../ \n")
        _SYS_STR("............(....`=-,,...`.........................(...;_,,-\" \n")
        _SYS_STR("............/.`~,......`-.................................../ \n")
        _SYS_STR(".............`~.*-,.....................................|,./...,__ \n")
        _SYS_STR(",,_..........}.>-._...................................|.......`=~-, \n")
        _SYS_STR(".....`=~-,__......`,................................. \n")
        _SYS_STR("...................`=~-,,.,........................... \n")
        _SYS_STR(".........................`:,,..........................`\n")
        _SYS_STR("...........................`=-,...............,%%`>--==`` \n")
        _SYS_STR(".................................._.........._,-%%...` \n")
        _SYS_STR("...................................,\n"));
    /* clang-format on */
  }

  static void ShowHelp(const hecl::SystemString& toolName) {
    /* Select tool's help-text streamer */
    HelpOutput::HelpFunc helpFunc = nullptr;
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
      LogModule.report(logvisor::Error, fmt(_SYS_STR("unrecognized tool '{}' - can't help")), toolName);
      return;
    }

    HelpOutput ho(helpFunc);
    ho.go();
  }

  hecl::SystemStringView toolName() const override { return _SYS_STR("help"sv); }

  int run() override {
    ShowHelp(m_info.args.front());
    return 0;
  }
};
