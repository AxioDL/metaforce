#pragma once

#include "ToolBase.hpp"
#include <cstdio>

#if _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

#include "hecl/MultiProgressPrinter.hpp"

class ToolExtract final : public ToolBase {
  hecl::Database::IDataSpec::ExtractPassInfo m_einfo;
  struct SpecExtractPass {
    const hecl::Database::DataSpecEntry* m_entry;
    std::unique_ptr<hecl::Database::IDataSpec> m_instance;
    SpecExtractPass(const hecl::Database::DataSpecEntry* entry, std::unique_ptr<hecl::Database::IDataSpec>&& instance)
    : m_entry(entry), m_instance(std::move(instance)) {}
    SpecExtractPass(const SpecExtractPass& other) = delete;
    SpecExtractPass(SpecExtractPass&& other) = default;
  };
  std::vector<SpecExtractPass> m_specPasses;
  std::vector<hecl::Database::IDataSpec::ExtractReport> m_reps;
  std::unique_ptr<hecl::Database::Project> m_fallbackProj;
  hecl::Database::Project* m_useProj = nullptr;

public:
  explicit ToolExtract(const ToolPassInfo& info) : ToolBase(info) {
    if (!m_info.args.size())
      LogModule.report(logvisor::Fatal, FMT_STRING("hecl extract needs a source path as its first argument"));

    if (!info.project) {
      std::string rootDir;

      if (info.output.empty()) {
        /* Get name from input file and init project there */
        std::string baseFile = info.args.front();
        size_t slashPos = baseFile.rfind('/');
        if (slashPos == std::string::npos)
          slashPos = baseFile.rfind('\\');
        if (slashPos != std::string::npos)
          baseFile.assign(baseFile.begin() + slashPos + 1, baseFile.end());
        size_t dotPos = baseFile.rfind('.');
        if (dotPos != std::string::npos)
          baseFile.assign(baseFile.begin(), baseFile.begin() + dotPos);

        if (baseFile.empty())
          LogModule.report(logvisor::Fatal, FMT_STRING("hecl extract must be ran within a project directory"));

        rootDir = info.cwd + baseFile;
      } else {
        if (hecl::PathRelative(info.output.c_str()))
          rootDir = info.cwd + info.output;
        else
          rootDir = info.output;
      }

      size_t ErrorRef = logvisor::ErrorCount;
      hecl::ProjectRootPath newProjRoot(rootDir);
      newProjRoot.makeDir();
      m_fallbackProj.reset(new hecl::Database::Project(newProjRoot));
      if (logvisor::ErrorCount > ErrorRef)
        LogModule.report(logvisor::Fatal, FMT_STRING("unable to init project at '{}'"), rootDir);
      LogModule.report(logvisor::Info, FMT_STRING("initialized project at '{}/.hecl'"), rootDir);
      m_useProj = m_fallbackProj.get();
    } else
      m_useProj = info.project;

    m_einfo.srcpath = m_info.args.front();
    m_einfo.force = info.force;
    m_einfo.extractArgs.reserve(info.args.size());
    auto it = info.args.cbegin();
    ++it;
    for (; it != info.args.cend(); ++it)
      m_einfo.extractArgs.push_back(*it);

    m_specPasses.reserve(hecl::Database::DATA_SPEC_REGISTRY.size());
    for (const hecl::Database::DataSpecEntry* entry : hecl::Database::DATA_SPEC_REGISTRY) {
      if (entry->m_factory) {
        auto ds = entry->m_factory(*m_useProj, hecl::Database::DataSpecTool::Extract);
        if (ds && ds->canExtract(m_einfo, m_reps))
          m_specPasses.emplace_back(entry, std::move(ds));
      }
    }
  }

  static void Help(HelpOutput& help) {
    help.secHead("NAME");
    help.beginWrap();
    help.wrap("hecl-extract - Extract objects from supported package/image formats\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl extract <packagefile> [<subnode>...]\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap("This command recursively extracts all or part of a dataspec-supported "
                  "package format. Each object is decoded to a working format and added to the project.\n\n");
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("<packagefile>[/<subnode>...]", "input file");
    help.beginWrap();
    help.wrap("Specifies the package file or disc image to source data from. "
                  "An optional subnode specifies a named hierarchical-node specific "
                      "to the game architecture (levels/areas).");
    help.endWrap();
  }

  std::string_view toolName() const override { return "extract"sv; }

  static void _recursivePrint(int level, hecl::Database::IDataSpec::ExtractReport& rep) {
    for (int l = 0; l < level; ++l)
      fmt::print(FMT_STRING("  "));
    if (XTERM_COLOR)
      fmt::print(FMT_STRING("" BOLD "{}" NORMAL ""), rep.name);
    else
      fmt::print(FMT_STRING("{}"), rep.name);

    if (rep.desc.size())
      fmt::print(FMT_STRING(" [{}]"), rep.desc);
    fmt::print(FMT_STRING("\n"));
    for (hecl::Database::IDataSpec::ExtractReport& child : rep.childOpts)
      _recursivePrint(level + 1, child);
  }

  int run() override {
    if (m_specPasses.empty()) {
      if (XTERM_COLOR)
        fmt::print(FMT_STRING("" RED BOLD "NOTHING TO EXTRACT" NORMAL "\n"));
      else
        fmt::print(FMT_STRING("NOTHING TO EXTRACT\n"));
      return 1;
    }

    if (XTERM_COLOR)
      fmt::print(FMT_STRING("" GREEN BOLD "ABOUT TO EXTRACT:" NORMAL "\n"));
    else
      fmt::print(FMT_STRING("ABOUT TO EXTRACT:\n"));

    for (hecl::Database::IDataSpec::ExtractReport& rep : m_reps) {
      _recursivePrint(0, rep);
      fmt::print(FMT_STRING("\n"));
    }
    fflush(stdout);

    if (continuePrompt()) {
      for (SpecExtractPass& ds : m_specPasses) {
        if (XTERM_COLOR)
          fmt::print(FMT_STRING("" MAGENTA BOLD "Using DataSpec {}:" NORMAL "\n"), ds.m_entry->m_name);
        else
          fmt::print(FMT_STRING("Using DataSpec {}:\n"), ds.m_entry->m_name);

        ds.m_instance->doExtract(m_einfo, {true});
        fmt::print(FMT_STRING("\n\n"));
      }
    }

    return 0;
  }
};
