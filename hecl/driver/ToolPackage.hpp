#ifndef CTOOL_PACKAGE
#define CTOOL_PACKAGE

#include <vector>
#include <string>
#include "ToolBase.hpp"
#include <stdio.h>

class ToolPackage final : public ToolBase
{
    std::vector<hecl::ProjectPath> m_selectedItems;
    std::unique_ptr<hecl::Database::Project> m_fallbackProj;
    hecl::Database::Project* m_useProj;
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
        if (!hecl::StrCmp(path.getLastComponent(), _S("!world.blend")))
            AddSelectedItem(path);
        else if (!hecl::StrCmp(path.getLastComponent(), _S("!original_ids.yaml")))
        {
            auto pathComps = path.getPathComponents();
            if (pathComps.size() == 2 && pathComps[0] != _S("out"))
                AddSelectedItem(path);
        }
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

        /* Directory with 2 components not "Shared" and no nested !world.blend files == General PAK */
        if (checkGeneral && origSize == m_selectedItems.size())
        {
            auto pathComps = path.getPathComponents();
            if (pathComps.size() == 2 && pathComps[0] != _S("out") && pathComps[1] != _S("Shared"))
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
                                         m_fallbackProj->getProjectRootPath().getAbsolutePath().c_str(),
                                         root.getAbsolutePath().c_str());

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

    ~ToolPackage()
    {
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
        help.wrap(_S("hecl package [-a] [-o <package-out>] [<input-dir>]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command initiates a packaging pass on the project database. Packaging ")
                  _S("is analogous to linking in software development. All objects necessary to ")
                  _S("generate a complete package are gathered, grouped, and indexed within a .upak file.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<dir>"), _S("input directory"));
        help.beginWrap();
        help.wrap(_S("Specifies a project subdirectory to root the resulting package from. ")
                  _S("If any dependent-files fall outside this subdirectory, they will implicitly ")
                  _S("be gathered and packaged.\n"));
        help.endWrap();

        help.optionHead(_S("-o <package-out>"), _S("output package file"));
        help.beginWrap();
        help.wrap(_S("Specifies a target path to write the package. If not specified, the package ")
                  _S("is written into <project-root>/out/<relative-input-dirs>/<input-dir>.upak\n"));
        help.endWrap();

        help.optionHead(_S("-a"), _S("auto cook"));
        help.beginWrap();
        help.wrap(_S("Any referenced objects that haven't already been cooked are automatically cooked as ")
                  _S("part of the packaging process. If this flag is omitted, the packaging process will abort.\n"));
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
            hecl::Printf(_S("%s\n"), item.getRelativePath().c_str());

        if (continuePrompt())
        {
            hecl::ClientProcess cp(m_info.verbosityLevel, m_fast, m_info.force);
            for (const hecl::ProjectPath& path : m_selectedItems)
            {
                if (!m_useProj->packagePath(path, {}, m_fast, &cp))
                    LogModule.report(logvisor::Error, _S("Unable to package %s"), path.getAbsolutePath().c_str());
            }
            cp.waitUntilComplete();
        }

        return 0;
    }
};

#endif // CTOOL_PACKAGE
