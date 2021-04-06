#pragma once

#include "ToolBase.hpp"
#include <cstdio>

class ToolInstallAddon final : public ToolBase {
public:
  explicit ToolInstallAddon(const ToolPassInfo& info) : ToolBase(info) {}

  int run() override {
    hecl::blender::SharedBlenderToken.getBlenderConnection();
    return 0;
  }

  static void Help(HelpOutput& help) {
    help.secHead(_SYS_STR("NAME"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl-installaddon - Installs embedded Blender addon into local Blender\n"));
    help.endWrap();

    help.secHead(_SYS_STR("SYNOPSIS"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl installaddon\n"));
    help.endWrap();

    help.secHead(_SYS_STR("DESCRIPTION"));
    help.beginWrap();
    help.wrap(_SYS_STR("Installs the hecl Blender addon into Blender. The path to the blender executable ")
              _SYS_STR("can be overridden by setting the BLENDER_BIN environment variable."));
    help.endWrap();
  }

  hecl::SystemStringView toolName() const override { return _SYS_STR("installaddon"sv); }
};
