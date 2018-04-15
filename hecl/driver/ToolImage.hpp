#ifndef CTOOL_IMAGE
#define CTOOL_IMAGE

#if HECL_HAS_NOD

#include <vector>
#include <string>
#include "ToolBase.hpp"
#include <cstdio>
#include "nod/DiscGCN.hpp"
#include "nod/DiscWii.hpp"
#include "athena/FileReader.hpp"

class ToolImage final : public ToolBase
{
    std::unique_ptr<hecl::Database::Project> m_fallbackProj;
    hecl::Database::Project* m_useProj;

public:
    ToolImage(const ToolPassInfo& info)
    : ToolBase(info), m_useProj(info.project)
    {
        if (!info.project)
            LogModule.report(logvisor::Fatal, "hecl image must be ran within a project directory");

        /* Scan args */
        if (info.args.size())
        {
            /* See if project path is supplied via args and use that over the getcwd one */
            for (const hecl::SystemString& arg : info.args)
            {
                if (arg.empty())
                    continue;

                hecl::SystemString subPath;
                hecl::ProjectRootPath root = hecl::SearchForProject(MakePathArgAbsolute(arg, info.cwd), subPath);

                if (root)
                {
                    if (!m_fallbackProj)
                    {
                        m_fallbackProj.reset(new hecl::Database::Project(root));
                        m_useProj = m_fallbackProj.get();
                        break;
                    }
                }
            }
        }
        if (!m_useProj)
            LogModule.report(logvisor::Fatal,
                             "hecl image must be ran within a project directory or "
                             "provided a path within a project");
    }

    ~ToolImage()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-image - Generate GameCube/Wii disc image from packaged files\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl image [<input-dir>]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command uses the current contents of `out` to generate a GameCube or ")
                  _S("Wii disc image. `hecl package` must have been run previously to be effective.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<input-dir>"), _S("input directory"));
        help.beginWrap();
        help.wrap(_S("Specifies a project subdirectory to root the resulting image from. ")
                  _S("Project must contain an out/sys and out/files directory to succeed.\n"));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("image");}

    int run()
    {
        if (XTERM_COLOR)
            hecl::Printf(_S("" GREEN BOLD "ABOUT TO IMAGE:" NORMAL "\n"));
        else
            hecl::Printf(_S("ABOUT TO IMAGE:\n"));

        hecl::Printf(_S("  %s\n"), m_useProj->getProjectRootPath().getAbsolutePath().data());
        fflush(stdout);

        if (continuePrompt())
        {
            hecl::ProjectPath outPath(m_useProj->getProjectWorkingPath(), _S("out"));
            if (!outPath.isDirectory())
            {
                LogModule.report(logvisor::Error, _S("%s is not a directory"), outPath.getAbsolutePath().data());
                return 1;
            }

            hecl::ProjectPath bootBinPath(outPath, _S("sys/boot.bin"));
            if (!bootBinPath.isFile())
            {
                LogModule.report(logvisor::Error, _S("%s is not a file"), bootBinPath.getAbsolutePath().data());
                return 1;
            }

            athena::io::FileReader r(bootBinPath.getAbsolutePath());
            if (r.hasError())
            {
                LogModule.report(logvisor::Error, _S("unable to open %s"), bootBinPath.getAbsolutePath().data());
                return 1;
            }
            std::string id = r.readString(6);
            r.close();

            hecl::SystemStringConv idView(id);
            hecl::SystemString fileOut = hecl::SystemString(outPath.getAbsolutePath()) + _S('/') + idView.c_str();
            hecl::MultiProgressPrinter printer(true);
            auto progFunc = [&printer](float totalProg, nod::SystemStringView fileName, size_t fileBytesXfered)
            {
                printer.print(fileName.data(), nullptr, totalProg);
            };
            if (id[0] == 'G')
            {
                fileOut += _S(".gcm");
                if (nod::DiscBuilderGCN::CalculateTotalSizeRequired(outPath.getAbsolutePath()) == -1)
                    return 1;
                LogModule.report(logvisor::Info, _S("Generating %s as GameCube image"), fileOut.c_str());
                nod::DiscBuilderGCN db(fileOut, progFunc);
                if (db.buildFromDirectory(outPath.getAbsolutePath()) != nod::EBuildResult::Success)
                    return 1;
            }
            else
            {
                fileOut += _S(".iso");
                bool dualLayer;
                if (nod::DiscBuilderWii::CalculateTotalSizeRequired(outPath.getAbsolutePath(), dualLayer) == -1)
                    return 1;
                LogModule.report(logvisor::Info, _S("Generating %s as %s-layer Wii image"), fileOut.c_str(),
                                 dualLayer ? _S("dual") : _S("single"));
                nod::DiscBuilderWii db(fileOut, dualLayer, progFunc);
                if (db.buildFromDirectory(outPath.getAbsolutePath()) != nod::EBuildResult::Success)
                    return 1;
            }
        }

        return 0;
    }
};

#endif

#endif // CTOOL_IMAGE
