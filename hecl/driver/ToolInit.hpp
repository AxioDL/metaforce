#pragma once

#include "ToolBase.hpp"
#include <cstdio>

class ToolInit final : public ToolBase {
  const std::string* m_dir = nullptr;

public:
  explicit ToolInit(const ToolPassInfo& info) : ToolBase(info) {
    hecl::Sstat theStat;
    const std::string* dir;
    if (info.args.size())
      dir = &info.args.front();
    else
      dir = &info.cwd;

    if (hecl::Stat(dir->c_str(), &theStat)) {
      hecl::MakeDir(dir->c_str());
      if (hecl::Stat(dir->c_str(), &theStat)) {
        LogModule.report(logvisor::Fatal, FMT_STRING("unable to stat '{}'"), *dir);
        return;
      }
    }
    if (!S_ISDIR(theStat.st_mode)) {
      LogModule.report(logvisor::Fatal, FMT_STRING("'{}' is not a directory"), *dir);
      return;
    }

    std::string testPath = *dir + "/.hecl/beacon";
    if (!hecl::Stat(testPath.c_str(), &theStat)) {
      LogModule.report(logvisor::Fatal, FMT_STRING("project already exists at '{}'"), *dir);
      return;
    }

    m_dir = dir;
  }

  int run() override {
    if (!m_dir)
      return 1;
    size_t ErrorRef = logvisor::ErrorCount;
    hecl::Database::Project proj((hecl::ProjectRootPath(*m_dir)));
    if (logvisor::ErrorCount > ErrorRef)
      return 1;
    LogModule.report(logvisor::Info, FMT_STRING("initialized project at '{}/.hecl'"), *m_dir);
    return 0;
  }

  static void Help(HelpOutput& help) {
    help.secHead("NAME");
    help.beginWrap();
    help.wrap("hecl-init - Initialize a brand-new project database\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl init [<dir>]\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap("Creates a ");
    help.wrapBold(".hecl");
    help.wrap(" directory within the selected directory with an initialized database index. "
                  "This constitutes an empty HECL project, ready for making stuff!!\n");
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("<dir>", "group directory path");
    help.beginWrap();
    help.wrap("Directory to create new project database in. If not specified, current directory is used.\n");
    help.endWrap();
  }

  std::string_view toolName() const override { return "init"sv; }
};
