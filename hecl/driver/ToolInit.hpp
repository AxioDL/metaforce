#pragma once

#include "ToolBase.hpp"
#include <cstdio>

class ToolInit final : public ToolBase {
  const hecl::SystemString* m_dir = NULL;

public:
  ToolInit(const ToolPassInfo& info) : ToolBase(info) {
    hecl::Sstat theStat;
    const hecl::SystemString* dir;
    if (info.args.size())
      dir = &info.args.front();
    else
      dir = &info.cwd;

    if (hecl::Stat(dir->c_str(), &theStat)) {
      hecl::MakeDir(dir->c_str());
      if (hecl::Stat(dir->c_str(), &theStat)) {
        LogModule.report(logvisor::Fatal, _SYS_STR("unable to stat '%s'"), dir->c_str());
        return;
      }
    }
    if (!S_ISDIR(theStat.st_mode)) {
      LogModule.report(logvisor::Fatal, _SYS_STR("'%s' is not a directory"), dir->c_str());
      return;
    }

    hecl::SystemString testPath = *dir + _SYS_STR("/.hecl/beacon");
    if (!hecl::Stat(testPath.c_str(), &theStat)) {
      LogModule.report(logvisor::Fatal, _SYS_STR("project already exists at '%s'"), dir->c_str());
      return;
    }

    m_dir = dir;
  }

  int run() {
    if (!m_dir)
      return 1;
    size_t ErrorRef = logvisor::ErrorCount;
    hecl::Database::Project proj((hecl::ProjectRootPath(*m_dir)));
    if (logvisor::ErrorCount > ErrorRef)
      return 1;
    LogModule.report(logvisor::Info, _SYS_STR("initialized project at '%s/.hecl'"), m_dir->c_str());
    return 0;
  }

  static void Help(HelpOutput& help) {
    help.secHead(_SYS_STR("NAME"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl-init - Initialize a brand-new project database\n"));
    help.endWrap();

    help.secHead(_SYS_STR("SYNOPSIS"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl init [<dir>]\n"));
    help.endWrap();

    help.secHead(_SYS_STR("DESCRIPTION"));
    help.beginWrap();
    help.wrap(_SYS_STR("Creates a "));
    help.wrapBold(_SYS_STR(".hecl"));
    help.wrap(_SYS_STR(" directory within the selected directory with an initialized database index. ")
                  _SYS_STR("This constitutes an empty HECL project, ready for making stuff!!\n"));
    help.endWrap();

    help.secHead(_SYS_STR("OPTIONS"));
    help.optionHead(_SYS_STR("<dir>"), _SYS_STR("group directory path"));
    help.beginWrap();
    help.wrap(_SYS_STR("Directory to create new project database in. If not specified, current directory is used.\n"));
    help.endWrap();
  }

  hecl::SystemString toolName() const { return _SYS_STR("init"); }
};
