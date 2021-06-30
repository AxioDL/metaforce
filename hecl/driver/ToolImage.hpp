#pragma once

#if HECL_HAS_NOD

#include <vector>
#include <string>
#include "ToolBase.hpp"
#include <cstdio>
#include "nod/DiscGCN.hpp"
#include "nod/DiscWii.hpp"
#include "athena/FileReader.hpp"

class ToolImage final : public ToolBase {
  std::unique_ptr<hecl::Database::Project> m_fallbackProj;
  hecl::Database::Project* m_useProj;

public:
  explicit ToolImage(const ToolPassInfo& info) : ToolBase(info), m_useProj(info.project) {
    if (!info.project)
      LogModule.report(logvisor::Fatal, FMT_STRING("hecl image must be ran within a project directory"));

    /* Scan args */
    if (info.args.size()) {
      /* See if project path is supplied via args and use that over the getcwd one */
      for (const std::string& arg : info.args) {
        if (arg.empty())
          continue;

        std::string subPath;
        hecl::ProjectRootPath root = hecl::SearchForProject(MakePathArgAbsolute(arg, info.cwd), subPath);

        if (root) {
          if (!m_fallbackProj) {
            m_fallbackProj.reset(new hecl::Database::Project(root));
            m_useProj = m_fallbackProj.get();
            break;
          }
        }
      }
    }
    if (!m_useProj)
      LogModule.report(logvisor::Fatal,
                       FMT_STRING("hecl image must be ran within a project directory or "
                           "provided a path within a project"));
  }

  ~ToolImage() override = default;

  static void Help(HelpOutput& help) {
    help.secHead("NAME");
    help.beginWrap();
    help.wrap("hecl-image - Generate GameCube/Wii disc image from packaged files\n");
    help.endWrap();

    help.secHead("SYNOPSIS");
    help.beginWrap();
    help.wrap("hecl image [<input-dir>]\n");
    help.endWrap();

    help.secHead("DESCRIPTION");
    help.beginWrap();
    help.wrap("This command uses the current contents of `out` to generate a GameCube or "
                  "Wii disc image. `hecl package` must have been run previously to be effective.\n");
    help.endWrap();

    help.secHead("OPTIONS");
    help.optionHead("<input-dir>", "input directory");
    help.beginWrap();
    help.wrap("Specifies a project subdirectory to root the resulting image from. "
                  "Project must contain an out/sys and out/files directory to succeed.\n");
    help.endWrap();
  }

  std::string_view toolName() const override { return "image"sv; }

  int run() override {
    if (XTERM_COLOR)
      fmt::print(FMT_STRING("" GREEN BOLD "ABOUT TO IMAGE:" NORMAL "\n"));
    else
      fmt::print(FMT_STRING("ABOUT TO IMAGE:\n"));

    fmt::print(FMT_STRING("  {}\n"), m_useProj->getProjectRootPath().getAbsolutePath());
    fflush(stdout);

    if (continuePrompt()) {
      hecl::ProjectPath outPath(m_useProj->getProjectWorkingPath(), "out");
      if (!outPath.isDirectory()) {
        LogModule.report(logvisor::Error, FMT_STRING("{} is not a directory"), outPath.getAbsolutePath());
        return 1;
      }

      hecl::ProjectPath bootBinPath(outPath, "sys/boot.bin");
      if (!bootBinPath.isFile()) {
        LogModule.report(logvisor::Error, FMT_STRING("{} is not a file"), bootBinPath.getAbsolutePath());
        return 1;
      }

      athena::io::FileReader r(bootBinPath.getAbsolutePath());
      if (r.hasError()) {
        LogModule.report(logvisor::Error, FMT_STRING("unable to open {}"), bootBinPath.getAbsolutePath());
        return 1;
      }
      std::string id = r.readString(6);
      r.close();

      std::string fileOut = std::string(outPath.getAbsolutePath()) + '/' + id;
      hecl::MultiProgressPrinter printer(true);
      auto progFunc = [&printer](float totalProg, std::string_view fileName, size_t fileBytesXfered) {
        printer.print(fileName, std::nullopt, totalProg);
      };
      if (id[0] == 'G') {
        fileOut += ".gcm";
        if (nod::DiscBuilderGCN::CalculateTotalSizeRequired(outPath.getAbsolutePath()) == UINT64_MAX)
          return 1;
        LogModule.report(logvisor::Info, FMT_STRING("Generating {} as GameCube image"), fileOut);
        nod::DiscBuilderGCN db(fileOut, progFunc);
        if (db.buildFromDirectory(outPath.getAbsolutePath()) != nod::EBuildResult::Success)
          return 1;
      } else {
        fileOut += ".iso";
        bool dualLayer;
        if (nod::DiscBuilderWii::CalculateTotalSizeRequired(outPath.getAbsolutePath(), dualLayer) == UINT64_MAX)
          return 1;
        LogModule.report(logvisor::Info, FMT_STRING("Generating {} as {}-layer Wii image"), fileOut,
                         dualLayer ? "dual" : "single");
        nod::DiscBuilderWii db(fileOut, dualLayer, progFunc);
        if (db.buildFromDirectory(outPath.getAbsolutePath()) != nod::EBuildResult::Success)
          return 1;
      }
    }

    return 0;
  }
};

#endif
