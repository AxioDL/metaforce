#ifndef CTOOL_PACKAGE
#define CTOOL_PACKAGE

#include <vector>
#include <string>
#include "ToolBase.hpp"
#include <cstdio>

class ToolPackage final : public ToolBase
{
    std::vector<hecl::ProjectPath> m_selectedItems;
    std::unique_ptr<hecl::Database::Project> m_fallbackProj;
    hecl::Database::Project* m_useProj;
    const hecl::Database::DataSpecEntry* m_spec = nullptr;
    bool m_fast = false;

    void AddSelectedItem(const hecl::ProjectPath& path)
    {
        for (const hecl::ProjectPath& item : m_selectedItems)
            if (item == path)
                return;
        m_selectedItems.push_back(path);
    }

    void CheckFile(const hecl::ProjectPath& path)
    {
        if (!hecl::StrCmp(path.getLastComponent().data(), _S("!world.blend")))
            AddSelectedItem(path);
#if RUNTIME_ORIGINAL_IDS
        else if (!hecl::StrCmp(path.getLastComponent().data(), _S("!original_ids.yaml")))
        {
            auto pathComps = path.getPathComponents();
            if (pathComps.size() == 2 && pathComps[0] != _S("out"))
                AddSelectedItem(path);
        }
#endif
    }

    void FindSelectedItems(const hecl::ProjectPath& path, bool checkGeneral)
    {
        if (path.isFile())
        {
            CheckFile(path);
            return;
        }

        size_t origSize = m_selectedItems.size();
        hecl::DirectoryEnumerator dEnum(path.getAbsolutePath(),
                                        hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
        for (const auto& ent : dEnum)
        {
            hecl::ProjectPath childPath(path, ent.m_name);
            if (ent.m_isDir)
                FindSelectedItems(childPath, checkGeneral && childPath.getPathComponents().size() <= 2);
            else
                CheckFile(childPath);
        }

        /* Directory with 2 components not "Shared" or macOS app bundle
         * and no nested !world.blend files == General PAK */
        if (checkGeneral && origSize == m_selectedItems.size())
        {
            auto pathComps = path.getPathComponents();
            if (pathComps.size() == 2 && pathComps[0] != _S("out") &&
                pathComps[1] != _S("Shared") && pathComps[0].find(_S(".app")) == hecl::SystemString::npos)
                AddSelectedItem(path);
        }
    }

public:
    ToolPackage(const ToolPassInfo& info)
    : ToolBase(info), m_useProj(info.project)
    {
        if (!info.project)
            LogModule.report(logvisor::Fatal, "hecl package must be ran within a project directory");

        /* Scan args */
        if (info.args.size())
        {
            /* See if project path is supplied via args and use that over the getcwd one */
            m_selectedItems.reserve(info.args.size());
            for (const hecl::SystemString& arg : info.args)
            {
                if (arg.empty())
                    continue;
                else if (!arg.compare(_S("--fast")))
                {
                    m_fast = true;
                    continue;
                }
                else if (arg.size() >= 8 && !arg.compare(0, 7, _S("--spec=")))
                {
                    hecl::SystemString specName(arg.begin() + 7, arg.end());
                    for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY)
                    {
                        if (!hecl::StrCaseCmp(spec->m_name.data(), specName.c_str()))
                        {
                            m_spec = spec;
                            break;
                        }
                    }
                    if (!m_spec)
                        LogModule.report(logvisor::Fatal, "unable to find data spec '%s'", specName.c_str());
                    continue;
                }
                else if (arg.size() >= 2 && arg[0] == _S('-') && arg[1] == _S('-'))
                    continue;

                hecl::SystemString subPath;
                hecl::ProjectRootPath root = hecl::SearchForProject(MakePathArgAbsolute(arg, info.cwd), subPath);

                if (root)
                {
                    if (!m_fallbackProj)
                    {
                        m_fallbackProj.reset(new hecl::Database::Project(root));
                        m_useProj = m_fallbackProj.get();
                    }
                    else if (m_fallbackProj->getProjectRootPath() != root)
                        LogModule.report(logvisor::Fatal,
                                         _S("hecl package can only process multiple items in the same project; ")
                                         _S("'%s' and '%s' are different projects"),
                                         m_fallbackProj->getProjectRootPath().getAbsolutePath().data(),
                                         root.getAbsolutePath().data());

                    FindSelectedItems({*m_useProj, subPath}, true);
                }
            }
        }
        if (!m_useProj)
            LogModule.report(logvisor::Fatal,
                             "hecl package must be ran within a project directory or "
                             "provided a path within a project");

        /* Default case: recursive at root */
        if (m_selectedItems.empty())
            FindSelectedItems({*m_useProj, _S("")}, true);
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-pack\n")
                  _S("hecl-package - Package objects within the project database\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl package [--spec=<spec>] [<input-dir>]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command initiates a packaging pass on the project database. Packaging ")
                  _S("is analogous to linking in software development. All objects necessary to ")
                  _S("generate a complete package are gathered, grouped, and indexed within a .upak file.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("--spec=<spec>"), _S("data specification"));
        help.beginWrap();
        help.wrap(_S("Specifies a DataSpec to use when cooking and generating the package. ")
                  _S("This build of hecl supports the following values of <spec>:\n"));
        for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY)
        {
            if (!spec->m_factory)
                continue;
            help.wrap(_S("  "));
            help.wrapBold(spec->m_name.data());
            help.wrap(_S("\n"));
        }
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<input-dir>"), _S("input directory"));
        help.beginWrap();
        help.wrap(_S("Specifies a project subdirectory to root the resulting package from. ")
                  _S("If any dependent files fall outside this subdirectory, they will be implicitly ")
                  _S("gathered and packaged.\n"));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("package");}

    int run()
    {
        if (XTERM_COLOR)
            hecl::Printf(_S("" GREEN BOLD "ABOUT TO PACKAGE:" NORMAL "\n"));
        else
            hecl::Printf(_S("ABOUT TO PACKAGE:\n"));

        for (auto& item : m_selectedItems)
            hecl::Printf(_S("  %s\n"), item.getRelativePath().data());
        fflush(stdout);

        if (continuePrompt())
        {
            hecl::MultiProgressPrinter printer(true);
            hecl::ClientProcess cp(&printer);
            for (const hecl::ProjectPath& path : m_selectedItems)
            {
                if (!m_useProj->packagePath(path, printer, m_fast, m_spec, &cp))
                    LogModule.report(logvisor::Error, _S("Unable to package %s"), path.getAbsolutePath().data());
            }
            cp.waitUntilComplete();
        }

        return 0;
    }

    void cancel()
    {
        m_useProj->interruptCook();
    }
};

#endif // CTOOL_PACKAGE
