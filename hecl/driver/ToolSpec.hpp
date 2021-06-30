#pragma once

#include "ToolBase.hpp"
#include <cstdio>
#include <map>

class ToolSpec final : public ToolBase {
  enum Mode { MLIST = 0, MENABLE, MDISABLE } mode = MLIST;

public:
  explicit ToolSpec(const ToolPassInfo& info) : ToolBase(info) {
    if (info.args.empty())
      return;

    if (!info.project)
      LogModule.report(logvisor::Fatal, FMT_STRING("hecl spec must be ran within a project directory"));

    const auto& specs = info.project->getDataSpecs();
    std::string firstArg = info.args.front();
    hecl::ToLower(firstArg);

    if (firstArg == "enable")
      mode = MENABLE;
    else if (firstArg == "disable")
      mode = MDISABLE;
    else
      return;

    if (info.args.size() < 2)
      LogModule.report(logvisor::Fatal, FMT_STRING("Speclist argument required"));

    auto it = info.args.begin();
    ++it;
    for (; it != info.args.end(); ++it) {

      bool found = false;
      for (auto& spec : specs) {
        if (!it->compare(spec.spec.m_name)) {
          found = true;
          break;
        }
      }
      if (!found)
        LogModule.report(logvisor::Fatal, FMT_STRING("'{}' is not found in the dataspec registry"), *it);
    }
  }

  static void Help(HelpOutput& help) {
    help.secHead("NAME");
    help.beginWrap();
    help.wrap("hecl-spec - Configure target data options\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl spec [enable|disable] [<specname>...]\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap(
        "This command configures the HECL project with the user's preferred target DataSpecs.\n\n"
            "Providing enable/disable argument will bulk-set the enable status of the provided spec(s)"
                "list. If enable/disable is not provided, a list of supported DataSpecs is printed.\n\n");
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("<specname>...", "DataSpec name(s)");
    help.beginWrap();
    help.wrap("Specifies platform-names to enable/disable");
    help.endWrap();
  }

  std::string_view toolName() const override { return "spec"sv; }

  int run() override {
    if (!m_info.project) {
      for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
        if (XTERM_COLOR)
          fmt::print(FMT_STRING("" BOLD CYAN "{}" NORMAL "\n"), spec->m_name);
        else
          fmt::print(FMT_STRING("{}\n"), spec->m_name);
        fmt::print(FMT_STRING("  {}\n"), spec->m_desc);
      }
      return 0;
    }

    const auto& specs = m_info.project->getDataSpecs();
    if (mode == MLIST) {
      for (auto& spec : specs) {
        if (XTERM_COLOR)
          fmt::print(FMT_STRING("" BOLD CYAN "{}" NORMAL ""), spec.spec.m_name);
        else
          fmt::print(FMT_STRING("{}"), spec.spec.m_name);
        if (spec.active) {
          if (XTERM_COLOR)
            fmt::print(FMT_STRING(" " BOLD GREEN "[ENABLED]" NORMAL ""));
          else
            fmt::print(FMT_STRING(" [ENABLED]"));
        }
        fmt::print(FMT_STRING("\n  {}\n"), spec.spec.m_desc);
      }
      return 0;
    }

    std::vector<std::string> opSpecs;
    auto it = m_info.args.begin();
    ++it;
    for (; it != m_info.args.end(); ++it) {
      std::string itName = *it;
      hecl::ToLower(itName);
      for (auto& spec : specs) {
        std::string compName(spec.spec.m_name);
        hecl::ToLower(compName);
        if (itName == compName) {
          opSpecs.emplace_back(spec.spec.m_name);
          break;
        }
      }
    }

    if (opSpecs.size()) {
      if (mode == MENABLE)
        m_info.project->enableDataSpecs(opSpecs);
      else if (mode == MDISABLE)
        m_info.project->disableDataSpecs(opSpecs);
    }

    return 0;
  }
};
