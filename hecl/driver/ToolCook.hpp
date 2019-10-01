#pragma once

#include "ToolBase.hpp"
#include <cstdio>
#include "hecl/ClientProcess.hpp"

class ToolCook final : public ToolBase {
  std::vector<hecl::ProjectPath> m_selectedItems;
  std::unique_ptr<hecl::Database::Project> m_fallbackProj;
  hecl::Database::Project* m_useProj;
  const hecl::Database::DataSpecEntry* m_spec = nullptr;
  bool m_recursive = false;
  bool m_fast = false;

public:
  explicit ToolCook(const ToolPassInfo& info) : ToolBase(info), m_useProj(info.project) {
    /* Check for recursive flag */
    for (hecl::SystemChar arg : info.flags)
      if (arg == _SYS_STR('r'))
        m_recursive = true;

    /* Scan args */
    if (info.args.size()) {
      /* See if project path is supplied via args and use that over the getcwd one */
      m_selectedItems.reserve(info.args.size());
      for (const hecl::SystemString& arg : info.args) {
        if (arg.empty())
          continue;
        else if (arg == _SYS_STR("--fast")) {
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
                             fmt(_SYS_STR("hecl cook can only process multiple items in the same project; ")
                                 _SYS_STR("'{}' and '{}' are different projects")),
                             m_fallbackProj->getProjectRootPath().getAbsolutePath(),
                             root.getAbsolutePath());
          m_selectedItems.emplace_back(*m_useProj, subPath);
        }
      }
    }
    if (!m_useProj)
      LogModule.report(logvisor::Fatal,
                       fmt("hecl cook must be ran within a project directory or "
                           "provided a path within a project"));

    /* Default case: recursive at root */
    if (m_selectedItems.empty()) {
      m_selectedItems.reserve(1);
      m_selectedItems.push_back({hecl::ProjectPath(*m_useProj, _SYS_STR(""))});
      m_recursive = true;
    }
  }

  static void Help(HelpOutput& help) {
    help.secHead(_SYS_STR("NAME"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl-cook - Cook objects within the project database\n"));
    help.endWrap();

    help.secHead(_SYS_STR("SYNOPSIS"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl cook [-rf] [--fast] [--spec=<spec>] [<pathspec>...]\n"));
    help.endWrap();

    help.secHead(_SYS_STR("DESCRIPTION"));
    help.beginWrap();
    help.wrap(_SYS_STR("This command initiates a cooking pass on the project database. Cooking ")
                  _SYS_STR("is analogous to compiling in software development. The resulting object buffers ")
                      _SYS_STR("are cached within the project database. HECL performs the following ")
                          _SYS_STR("tasks for each object during the cook process:\n\n"));
    help.wrapBold(_SYS_STR("- Object Gather: "));
    help.wrap(_SYS_STR("Files added with "));
    help.wrapBold(_SYS_STR("hecl add"));
    help.wrap(_SYS_STR(" are queried for their dependent files (e.g. "));
    help.wrapBold(_SYS_STR(".blend"));
    help.wrap(_SYS_STR(" files return any linked "));
    help.wrapBold(_SYS_STR(".png"));
    help.wrap(_SYS_STR(" images). If the dependent files are unable to be found, the cook process aborts.\n\n"));
    help.wrapBold(_SYS_STR("- Modtime Comparison: "));
    help.wrap(_SYS_STR("Files that have previously finished a cook pass are inspected for their time of ")
                  _SYS_STR("last modification. If the file hasn't changed since its previous cook-pass, the ") _SYS_STR(
                      "process is skipped. If the file has been moved or deleted, the object is automatically ")
                      _SYS_STR("removed from the project database.\n\n"));
    help.wrapBold(_SYS_STR("- Cook: "));
    help.wrap(_SYS_STR("A type-specific procedure compiles the file's contents into an efficient format ")
                  _SYS_STR("for use by the runtime. A data-buffer is provided to HECL.\n\n"));
    help.wrapBold(_SYS_STR("- Hash and Compress: "));
    help.wrap(_SYS_STR("The data-buffer is hashed and compressed before being cached in the object database.\n\n"));
    help.endWrap();

    help.secHead(_SYS_STR("OPTIONS"));
    help.optionHead(_SYS_STR("<pathspec>..."), _SYS_STR("input file(s)"));
    help.beginWrap();
    help.wrap(_SYS_STR("Specifies working file(s) containing production data to be cooked by HECL. ")
                  _SYS_STR("Glob-strings may be specified (e.g. "));
    help.wrapBold(_SYS_STR("*.blend"));
    help.wrap(_SYS_STR(") to automatically cook all matching current-directory files in the project database. ")
                  _SYS_STR("If no path specified, all files in the project database are cooked.\n"));
    help.endWrap();

    help.optionHead(_SYS_STR("-r"), _SYS_STR("recursion"));
    help.beginWrap();
    help.wrap(_SYS_STR("Enables recursive file-matching for cooking entire directories of working files.\n"));
    help.endWrap();
    help.optionHead(_SYS_STR("-f"), _SYS_STR("force"));
    help.beginWrap();
    help.wrap(_SYS_STR("Forces cooking of all matched files, ignoring timestamp differences.\n"));
    help.endWrap();
    help.optionHead(_SYS_STR("--fast"), _SYS_STR("fast cook"));
    help.beginWrap();
    help.wrap(_SYS_STR("Performs draft-optimization cooking for supported data types.\n"));
    help.endWrap();

    help.optionHead(_SYS_STR("--spec=<spec>"), _SYS_STR("data specification"));
    help.beginWrap();
    help.wrap(_SYS_STR("Specifies a DataSpec to use when cooking. ")
                  _SYS_STR("This build of hecl supports the following values of <spec>:\n"));
    for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
      if (!spec->m_factory)
        continue;
      help.wrap(_SYS_STR("  "));
      help.wrapBold(spec->m_name.data());
      help.wrap(_SYS_STR("\n"));
    }
  }

  hecl::SystemString toolName() const override { return _SYS_STR("cook"); }

  int run() override {
    hecl::MultiProgressPrinter printer(true);
    hecl::ClientProcess cp(&printer);
    for (const hecl::ProjectPath& path : m_selectedItems)
      m_useProj->cookPath(path, printer, m_recursive, m_info.force, m_fast, m_spec, &cp);
    cp.waitUntilComplete();
    return 0;
  }

  void cancel() override { m_useProj->interruptCook(); }
};
