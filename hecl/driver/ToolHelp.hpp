#pragma once

#include "ToolBase.hpp"
#include <cstdio>
#include <functional>

class ToolHelp final : public ToolBase {

public:
  explicit ToolHelp(const ToolPassInfo& info) : ToolBase(info) {
    if (m_info.args.empty()) {
      LogModule.report(logvisor::Error, FMT_STRING("help requires a tool name argument"));
      return;
    }
    m_good = true;
  }

  ~ToolHelp() override = default;

  static void Help(HelpOutput& help) {
    /* clang-format off */
    help.printBold(
        "                                ___________ \n"
        "                           ,.-'\"...........``~., \n"
        "                        ,.-\".......................\"-., \n"
        "                    ,/..................................\":, \n"
        "                 .,?........................................, \n"
        "                /...........................................,}\n"
        "             ./........................................,:`^`..}\n"
        "            ./.......................................,:\"...../\n"
        "            ?.....__..................................:`...../\n"
        "           /__.(...\"~-,_...........................,:`....../\n"
        "          /(_....\"~,_....\"~,_.....................,:`...._/ \n"
        "          {.._$;_....\"=,_.....\"-,_......,.-~-,},.~\";/....} \n"
        "           ((...*~_......\"=-._...\";,,./`........../\"..../ \n"
        "   ,,,___.`~,......\"~.,....................`......}....../ \n"
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
    /* clang-format on */
  }

  static void ShowHelp(const std::string& toolName) {
    /* Select tool's help-text streamer */
    HelpOutput::HelpFunc helpFunc = nullptr;
    if (toolName == "init")
      helpFunc = ToolInit::Help;
    else if (toolName == "spec")
      helpFunc = ToolSpec::Help;
    else if (toolName == "extract")
      helpFunc = ToolExtract::Help;
    else if (toolName == "cook")
      helpFunc = ToolCook::Help;
    else if (toolName == "package" || toolName == "pack")
      helpFunc = ToolPackage::Help;
    else if (toolName == "help")
      helpFunc = ToolHelp::Help;
    else {
      LogModule.report(logvisor::Error, FMT_STRING("unrecognized tool '{}' - can't help"), toolName);
      return;
    }

    HelpOutput ho(helpFunc);
    ho.go();
  }

  std::string_view toolName() const override { return "help"sv; }

  int run() override {
    ShowHelp(m_info.args.front());
    return 0;
  }
};
