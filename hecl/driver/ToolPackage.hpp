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
    auto lastComp = path.getLastComponent();
    if (hecl::StringUtils::BeginsWith(lastComp, "!world") &&
        hecl::StringUtils::EndsWith(lastComp, ".blend"))
      AddSelectedItem(path);
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
      if (pathComps.size() == 2 && pathComps[0] != "out" && pathComps[1] != "Shared" &&
          pathComps[0].find(".app") == std::string::npos)
        AddSelectedItem(path);
    }
  }

public:
  explicit ToolPackage(const ToolPassInfo& info) : ToolBase(info), m_useProj(info.project) {
    if (!info.project)
      LogModule.report(logvisor::Fatal, FMT_STRING("hecl package must be ran within a project directory"));

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
                             FMT_STRING("hecl package can only process multiple items in the same project; "
                                 "'{}' and '{}' are different projects"),
                             m_fallbackProj->getProjectRootPath().getAbsolutePath(),
                             root.getAbsolutePath());

          FindSelectedItems({*m_useProj, subPath}, true);
        }
      }
    }
    if (!m_useProj)
      LogModule.report(logvisor::Fatal,
                       FMT_STRING("hecl package must be ran within a project directory or "
                           "provided a path within a project"));

    /* Default case: recursive at root */
    if (m_selectedItems.empty())
      FindSelectedItems({*m_useProj, ""}, true);
  }

  static void Help(HelpOutput& help) {
    help.secHead("NAME");
    help.beginWrap();
    help.wrap(
        "hecl-pack\n"
        "hecl-package - Package objects within the project database\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl package [--spec=<spec>] [<input-dir>]\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap(
        "This command initiates a packaging pass on the project database. Packaging "
        "is analogous to linking in software development. All objects necessary to "
        "generate a complete package are gathered, grouped, and indexed within a .upak file.\n");
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("--spec=<spec>", "data specification");
    help.beginWrap();
    help.wrap(
        "Specifies a DataSpec to use when cooking and generating the package. "
        "This build of hecl supports the following values of <spec>:\n");
    for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY) {
      if (!spec->m_factory)
        continue;
      help.wrap("  ");
      help.wrapBold(spec->m_name.data());
      help.wrap("\n");
    }
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("<input-dir>", "input directory");
    help.beginWrap();
    help.wrap(
        "Specifies a project subdirectory to root the resulting package from. "
        "If any dependent files fall outside this subdirectory, they will be implicitly "
        "gathered and packaged.\n");
    help.endWrap();
  }

  std::string_view toolName() const override { return "package"sv; }

  int run() override {
    if (XTERM_COLOR)
      fmt::print(FMT_STRING("" GREEN BOLD "ABOUT TO PACKAGE:" NORMAL "\n"));
    else
      fmt::print(FMT_STRING("ABOUT TO PACKAGE:\n"));

    for (auto& item : m_selectedItems)
      fmt::print(FMT_STRING("  {}\n"), item.getRelativePath());
    fflush(stdout);

    if (continuePrompt()) {
      hecl::MultiProgressPrinter printer(true);
      hecl::ClientProcess cp(&printer);
      for (const hecl::ProjectPath& path : m_selectedItems) {
        if (!m_useProj->packagePath(path, printer, m_fast, m_spec, &cp))
          LogModule.report(logvisor::Error, FMT_STRING("Unable to package {}"), path.getAbsolutePath());
      }
      cp.waitUntilComplete();
    }

    return 0;
  }

  void cancel() override { m_useProj->interruptCook(); }
};
