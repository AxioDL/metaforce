#pragma once

#include <vector>
#include <string>
#include "ToolBase.hpp"
#include <cstdio>

class ToolPackage final : public ToolBase {
  std::vector<hecl::ProjectPath> m_selectedItems;
  std::unique_ptr<hecl::Database::Project> m_fallbackProj;
  hecl::Database::Project* m_useProj;
  const hecl::Database::DataSpecEntry* m_spec = nullptr;
  bool m_fast = false;

  void AddSelectedItem(const hecl::ProjectPath& path) {
    for (const hecl::ProjectPath& item : m_selectedItems)
      if (item == path)
        return;
    m_selectedItems.push_back(path);
  }

  void CheckFile(const hecl::ProjectPath& path) {
    if (!hecl::StrCmp(path.getLastComponent().data(), _SYS_STR("!world.blend")))
      AddSelectedItem(path);
#if RUNTIME_ORIGINAL_IDS
    else if (!hecl::StrCmp(path.getLastComponent().data(), _SYS_STR("!original_ids.yaml"))) {
      auto pathComps = path.getPathComponents();
      if (pathComps.size() == 2 && pathComps[0] != _SYS_STR("out"))
        AddSelectedItem(path);
    }
#endif
  }

  void FindSelectedItems(const hecl::ProjectPath& path, bool checkGeneral) {
    if (path.isFile()) {
      CheckFile(path);
      return;
    }

    size_t origSize = m_selectedItems.size();
    hecl::DirectoryEnumerator dEnum(path.getAbsolutePath(), hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false,
                                    false, true);
    for (const auto& ent : dEnum) {
      hecl::ProjectPath childPath(path, ent.m_name);
      if (ent.m_isDir)
        FindSelectedItems(childPath, checkGeneral && childPath.getPathComponents().size() <= 2);
      else
        CheckFile(childPath);
    }

    /* Directory with 2 components not "Shared" or macOS app bundle
     * and no nested !world.blend files == General PAK */
    if (checkGeneral && origSize == m_selectedItems.size()) {
      auto pathComps = path.getPathComponents();
      if (pathComps.size() == 2 && pathComps[0] != _SYS_STR("out") && pathComps[1] != _SYS_STR("Shared") &&
          pathComps[0].find(_SYS_STR(".app")) == hecl::SystemString::npos)
        AddSelectedItem(path);
    }
  }

public:
  ToolPackage(const ToolPassInfo& info) : ToolBase(info), m_useProj(info.project) {
    if (!info.project)
      LogModule.report(logvisor::Fatal, fmt("hecl package must be ran within a project directory"));

    /* Scan args */
    if (info.args.size()) {
      /* See if project path is supplied via args and use that over the getcwd one */
      m_selectedItems.reserve(info.args.size());
      for (const hecl::SystemString& arg : info.args) {
        if (arg.empty())
          continue;
        else if (!arg.compare(_SYS_STR("--fast"))) {
          m_fast = true;
          continue;
        } else if (arg.size() >= 8 && !arg.compare(0, 7, _SYS_STR("--spec="))) {
          hecl::SystemString specName(arg.begin() + 7, arg.end());
          for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
            if (!hecl::StrCaseCmp(spec->m_name.data(), specName.c_str())) {
              m_spec = spec;
              break;
            }
          }
          if (!m_spec)
            LogModule.report(logvisor::Fatal, fmt(_SYS_STR("unable to find data spec '{}'")), specName);
          continue;
        } else if (arg.size() >= 2 && arg[0] == _SYS_STR('-') && arg[1] == _SYS_STR('-'))
          continue;

        hecl::SystemString subPath;
        hecl::ProjectRootPath root = hecl::SearchForProject(MakePathArgAbsolute(arg, info.cwd), subPath);

        if (root) {
          if (!m_fallbackProj) {
            m_fallbackProj.reset(new hecl::Database::Project(root));
            m_useProj = m_fallbackProj.get();
          } else if (m_fallbackProj->getProjectRootPath() != root)
            LogModule.report(logvisor::Fatal,
                             fmt(_SYS_STR("hecl package can only process multiple items in the same project; ")
                                 _SYS_STR("'{}' and '{}' are different projects")),
                             m_fallbackProj->getProjectRootPath().getAbsolutePath(),
                             root.getAbsolutePath());

          FindSelectedItems({*m_useProj, subPath}, true);
        }
      }
    }
    if (!m_useProj)
      LogModule.report(logvisor::Fatal,
                       fmt("hecl package must be ran within a project directory or "
                           "provided a path within a project"));

    /* Default case: recursive at root */
    if (m_selectedItems.empty())
      FindSelectedItems({*m_useProj, _SYS_STR("")}, true);
  }

  static void Help(HelpOutput& help) {
    help.secHead(_SYS_STR("NAME"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl-pack\n") _SYS_STR("hecl-package - Package objects within the project database\n"));
    help.endWrap();

    help.secHead(_SYS_STR("SYNOPSIS"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl package [--spec=<spec>] [<input-dir>]\n"));
    help.endWrap();

    help.secHead(_SYS_STR("DESCRIPTION"));
    help.beginWrap();
    help.wrap(_SYS_STR("This command initiates a packaging pass on the project database. Packaging ")
                  _SYS_STR("is analogous to linking in software development. All objects necessary to ") _SYS_STR(
                      "generate a complete package are gathered, grouped, and indexed within a .upak file.\n"));
    help.endWrap();

    help.secHead(_SYS_STR("OPTIONS"));
    help.optionHead(_SYS_STR("--spec=<spec>"), _SYS_STR("data specification"));
    help.beginWrap();
    help.wrap(_SYS_STR("Specifies a DataSpec to use when cooking and generating the package. ")
                  _SYS_STR("This build of hecl supports the following values of <spec>:\n"));
    for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
      if (!spec->m_factory)
        continue;
      help.wrap(_SYS_STR("  "));
      help.wrapBold(spec->m_name.data());
      help.wrap(_SYS_STR("\n"));
    }
    help.endWrap();

    help.secHead(_SYS_STR("OPTIONS"));
    help.optionHead(_SYS_STR("<input-dir>"), _SYS_STR("input directory"));
    help.beginWrap();
    help.wrap(_SYS_STR("Specifies a project subdirectory to root the resulting package from. ")
                  _SYS_STR("If any dependent files fall outside this subdirectory, they will be implicitly ")
                      _SYS_STR("gathered and packaged.\n"));
    help.endWrap();
  }

  hecl::SystemString toolName() const { return _SYS_STR("package"); }

  int run() {
    if (XTERM_COLOR)
      fmt::print(fmt(_SYS_STR("" GREEN BOLD "ABOUT TO PACKAGE:" NORMAL "\n")));
    else
      fmt::print(fmt(_SYS_STR("ABOUT TO PACKAGE:\n")));

    for (auto& item : m_selectedItems)
      fmt::print(fmt(_SYS_STR("  {}\n")), item.getRelativePath());
    fflush(stdout);

    if (continuePrompt()) {
      hecl::MultiProgressPrinter printer(true);
      hecl::ClientProcess cp(&printer);
      for (const hecl::ProjectPath& path : m_selectedItems) {
        if (!m_useProj->packagePath(path, printer, m_fast, m_spec, &cp))
          LogModule.report(logvisor::Error, fmt(_SYS_STR("Unable to package {}")), path.getAbsolutePath());
      }
      cp.waitUntilComplete();
    }

    return 0;
  }

  void cancel() { m_useProj->interruptCook(); }
};
