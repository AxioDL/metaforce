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
    help.secHead("NAME");
    help.beginWrap();
    help.wrap("hecl-installaddon - Installs embedded Blender addon into local Blender\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl installaddon\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap("Installs the hecl Blender addon into Blender. The path to the blender executable "
              "can be overridden by setting the BLENDER_BIN environment variable.");
    help.endWrap();
  }

  std::string_view toolName() const override { return "installaddon"sv; }
};
