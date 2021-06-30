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
    for (char arg : info.flags)
      if (arg == 'r')
        m_recursive = true;

    /* Scan args */
    if (info.args.size()) {
      /* See if project path is supplied via args and use that over the getcwd one */
      m_selectedItems.reserve(info.args.size());
      for (const std::string& arg : info.args) {
        if (arg.empty())
          continue;
        else if (arg == "--fast") {
          m_fast = true;
          continue;
        } else if (arg.size() >= 8 && !arg.compare(0, 7, "--spec=")) {
          std::string specName(arg.begin() + 7, arg.end());
          for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
            if (!hecl::StrCaseCmp(spec->m_name.data(), specName.c_str())) {
              m_spec = spec;
              break;
            }
          }
          if (!m_spec)
            LogModule.report(logvisor::Fatal, FMT_STRING("unable to find data spec '{}'"), specName);
          continue;
        } else if (arg.size() >= 2 && arg[0] == '-' && arg[1] == '-')
          continue;

        std::string subPath;
        hecl::ProjectRootPath root = hecl::SearchForProject(MakePathArgAbsolute(arg, info.cwd), subPath);
        if (root) {
          if (!m_fallbackProj) {
            m_fallbackProj.reset(new hecl::Database::Project(root));
            m_useProj = m_fallbackProj.get();
          } else if (m_fallbackProj->getProjectRootPath() != root)
            LogModule.report(logvisor::Fatal,
                             FMT_STRING("hecl cook can only process multiple items in the same project; "
                                 "'{}' and '{}' are different projects"),
                             m_fallbackProj->getProjectRootPath().getAbsolutePath(),
                             root.getAbsolutePath());
          m_selectedItems.emplace_back(*m_useProj, subPath);
        }
      }
    }
    if (!m_useProj)
      LogModule.report(logvisor::Fatal,
                       FMT_STRING("hecl cook must be ran within a project directory or "
                           "provided a path within a project"));

    /* Default case: recursive at root */
    if (m_selectedItems.empty()) {
      m_selectedItems.reserve(1);
      m_selectedItems.push_back({hecl::ProjectPath(*m_useProj, "")});
      m_recursive = true;
    }
  }

  static void Help(HelpOutput& help) {
    help.secHead("NAME");
    help.beginWrap();
    help.wrap("hecl-cook - Cook objects within the project database\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl cook [-rf] [--fast] [--spec=<spec>] [<pathspec>...]\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap(
        "This command initiates a cooking pass on the project database. Cooking "
        "is analogous to compiling in software development. The resulting object buffers "
        "are cached within the project database. HECL performs the following "
        "tasks for each object during the cook process:\n\n");
    help.wrapBold("- Object Gather: ");
    help.wrap("Files added with ");
    help.wrapBold("hecl add");
    help.wrap(" are queried for their dependent files (e.g. ");
    help.wrapBold(".blend");
    help.wrap(" files return any linked ");
    help.wrapBold(".png");
    help.wrap(" images). If the dependent files are unable to be found, the cook process aborts.\n\n");
    help.wrapBold("- Modtime Comparison: ");
    help.wrap(
        "Files that have previously finished a cook pass are inspected for their time of "
        "last modification. If the file hasn't changed since its previous cook-pass, the "
        "process is skipped. If the file has been moved or deleted, the object is automatically "
        "removed from the project database.\n\n");
    help.wrapBold("- Cook: ");
    help.wrap(
        "A type-specific procedure compiles the file's contents into an efficient format "
        "for use by the runtime. A data-buffer is provided to HECL.\n\n");
    help.wrapBold("- Hash and Compress: ");
    help.wrap("The data-buffer is hashed and compressed before being cached in the object database.\n\n");
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("<pathspec>...", "input file(s)");
    help.beginWrap();
    help.wrap(
        "Specifies working file(s) containing production data to be cooked by HECL. "
        "Glob-strings may be specified (e.g. ");
    help.wrapBold("*.blend");
    help.wrap(
        ") to automatically cook all matching current-directory files in the project database. "
        "If no path specified, all files in the project database are cooked.\n");
    help.endWrap();

    help.optionHead("-r", "recursion");
    help.beginWrap();
    help.wrap("Enables recursive file-matching for cooking entire directories of working files.\n");
    help.endWrap();
    help.optionHead("-f", "force");
    help.beginWrap();
    help.wrap("Forces cooking of all matched files, ignoring timestamp differences.\n");
    help.endWrap();
    help.optionHead("--fast", "fast cook");
    help.beginWrap();
    help.wrap("Performs draft-optimization cooking for supported data types.\n");
    help.endWrap();

    help.optionHead("--spec=<spec>", "data specification");
    help.beginWrap();
    help.wrap(
        "Specifies a DataSpec to use when cooking. "
        "This build of hecl supports the following values of <spec>:\n");
    for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
      if (!spec->m_factory)
        continue;
      help.wrap("  ");
      help.wrapBold(spec->m_name.data());
      help.wrap("\n");
    }
  }

  std::string_view toolName() const override { return "cook"sv; }

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
