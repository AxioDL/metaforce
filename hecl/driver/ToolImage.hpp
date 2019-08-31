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
      LogModule.report(logvisor::Fatal, fmt("hecl image must be ran within a project directory"));

    /* Scan args */
    if (info.args.size()) {
      /* See if project path is supplied via args and use that over the getcwd one */
      for (const hecl::SystemString& arg : info.args) {
        if (arg.empty())
          continue;

        hecl::SystemString subPath;
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
                       fmt("hecl image must be ran within a project directory or "
                           "provided a path within a project"));
  }

  ~ToolImage() override = default;

  static void Help(HelpOutput& help) {
    help.secHead(_SYS_STR("NAME"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl-image - Generate GameCube/Wii disc image from packaged files\n"));
    help.endWrap();

    help.secHead(_SYS_STR("SYNOPSIS"));
    help.beginWrap();
    help.wrap(_SYS_STR("hecl image [<input-dir>]\n"));
    help.endWrap();

    help.secHead(_SYS_STR("DESCRIPTION"));
    help.beginWrap();
    help.wrap(_SYS_STR("This command uses the current contents of `out` to generate a GameCube or ")
                  _SYS_STR("Wii disc image. `hecl package` must have been run previously to be effective.\n"));
    help.endWrap();

    help.secHead(_SYS_STR("OPTIONS"));
    help.optionHead(_SYS_STR("<input-dir>"), _SYS_STR("input directory"));
    help.beginWrap();
    help.wrap(_SYS_STR("Specifies a project subdirectory to root the resulting image from. ")
                  _SYS_STR("Project must contain an out/sys and out/files directory to succeed.\n"));
    help.endWrap();
  }

  hecl::SystemString toolName() const override { return _SYS_STR("image"); }

  int run() override {
    if (XTERM_COLOR)
      fmt::print(fmt(_SYS_STR("" GREEN BOLD "ABOUT TO IMAGE:" NORMAL "\n")));
    else
      fmt::print(fmt(_SYS_STR("ABOUT TO IMAGE:\n")));

    fmt::print(fmt(_SYS_STR("  {}\n")), m_useProj->getProjectRootPath().getAbsolutePath());
    fflush(stdout);

    if (continuePrompt()) {
      hecl::ProjectPath outPath(m_useProj->getProjectWorkingPath(), _SYS_STR("out"));
      if (!outPath.isDirectory()) {
        LogModule.report(logvisor::Error, fmt(_SYS_STR("{} is not a directory")), outPath.getAbsolutePath());
        return 1;
      }

      hecl::ProjectPath bootBinPath(outPath, _SYS_STR("sys/boot.bin"));
      if (!bootBinPath.isFile()) {
        LogModule.report(logvisor::Error, fmt(_SYS_STR("{} is not a file")), bootBinPath.getAbsolutePath());
        return 1;
      }

      athena::io::FileReader r(bootBinPath.getAbsolutePath());
      if (r.hasError()) {
        LogModule.report(logvisor::Error, fmt(_SYS_STR("unable to open {}")), bootBinPath.getAbsolutePath());
        return 1;
      }
      std::string id = r.readString(6);
      r.close();

      hecl::SystemStringConv idView(id);
      hecl::SystemString fileOut = hecl::SystemString(outPath.getAbsolutePath()) + _SYS_STR('/') + idView.c_str();
      hecl::MultiProgressPrinter printer(true);
      auto progFunc = [&printer](float totalProg, nod::SystemStringView fileName, size_t fileBytesXfered) {
        printer.print(fileName.data(), nullptr, totalProg);
      };
      if (id[0] == 'G') {
        fileOut += _SYS_STR(".gcm");
        if (nod::DiscBuilderGCN::CalculateTotalSizeRequired(outPath.getAbsolutePath()) == UINT64_MAX)
          return 1;
        LogModule.report(logvisor::Info, fmt(_SYS_STR("Generating {} as GameCube image")), fileOut);
        nod::DiscBuilderGCN db(fileOut, progFunc);
        if (db.buildFromDirectory(outPath.getAbsolutePath()) != nod::EBuildResult::Success)
          return 1;
      } else {
        fileOut += _SYS_STR(".iso");
        bool dualLayer;
        if (nod::DiscBuilderWii::CalculateTotalSizeRequired(outPath.getAbsolutePath(), dualLayer) == UINT64_MAX)
          return 1;
        LogModule.report(logvisor::Info, fmt(_SYS_STR("Generating {} as {}-layer Wii image")), fileOut,
                         dualLayer ? _SYS_STR("dual") : _SYS_STR("single"));
        nod::DiscBuilderWii db(fileOut, dualLayer, progFunc);
        if (db.buildFromDirectory(outPath.getAbsolutePath()) != nod::EBuildResult::Success)
          return 1;
      }
    }

    return 0;
  }
};

#endif
