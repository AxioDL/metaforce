#ifndef CTOOL_COOK
#define CTOOL_COOK

#include "ToolBase.hpp"
#include <stdio.h>

class ToolCook final : public ToolBase
{
    std::list<HECL::ProjectPath> m_selectedItems;
    std::unique_ptr<HECL::Database::Project> m_fallbackProj;
    HECL::Database::Project* m_useProj;
    bool m_recursive = false;
public:
    ToolCook(const ToolPassInfo& info)
    : ToolBase(info), m_useProj(info.project)
    {
        /* Scan args */
        if (info.args.size())
        {
            /* See if project path is supplied via args and use that over the getcwd one */
            for (const HECL::SystemString& arg : info.args)
            {
                if (arg.empty())
                    continue;
                if (arg.size() >= 2 && arg[0] == _S('-'))
                {
                    switch (arg[1])
                    {
                    case _S('r'):
                        m_recursive = true;
                        break;
                    default: break;
                    }
                    continue;
                }
                HECL::SystemString subPath;
                HECL::ProjectRootPath root = HECL::SearchForProject(MakePathArgAbsolute(arg, info.cwd), subPath);
                if (root)
                {
                    if (!m_fallbackProj)
                    {
                        m_fallbackProj.reset(new HECL::Database::Project(root));
                        m_useProj = m_fallbackProj.get();
                    }
                    else if (m_fallbackProj->getProjectRootPath() != root)
                        LogModule.report(LogVisor::FatalError,
                                         _S("hecl cook can only process multiple items in the same project; ")
                                         _S("'%s' and '%s' are different projects"),
                                         m_fallbackProj->getProjectRootPath().getAbsolutePath().c_str(),
                                         root.getAbsolutePath().c_str());
                    m_selectedItems.emplace_back(*m_useProj, subPath);
                }
            }
        }
        if (!m_useProj)
            LogModule.report(LogVisor::FatalError,
                             "hecl cook must be ran within a project directory or "
                             "provided a path within a project");

        /* Default case: recursive at root */
        if (m_selectedItems.empty())
            m_selectedItems.push_back({HECL::ProjectPath(*m_useProj, _S("."))});
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-cook - Cook objects within the project database\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl cook [-r] [<pathspec>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command initiates a cooking pass on the project database. Cooking ")
                  _S("is analogous to compiling in software development. The resulting object buffers ")
                  _S("are cached within the project database. HECL performs the following ")
                  _S("tasks for each object during the cook process:\n\n"));
        help.wrapBold(_S("- Object Gather: "));
        help.wrap(_S("Files added with "));
        help.wrapBold(_S("hecl add"));
        help.wrap(_S(" are queried for their dependent files (e.g. "));
        help.wrapBold(_S(".blend"));
        help.wrap(_S(" files return any linked "));
        help.wrapBold(_S(".png"));
        help.wrap(_S(" images). If the dependent files are unable to be found, the cook process aborts.\n\n"));
        help.wrapBold(_S("- Modtime Comparison: "));
        help.wrap(_S("Files that have previously finished a cook pass are inspected for their time of ")
                  _S("last modification. If the file hasn't changed since its previous cook-pass, the ")
                  _S("process is skipped. If the file has been moved or deleted, the object is automatically ")
                  _S("removed from the project database.\n\n"));
        help.wrapBold(_S("- Cook: "));
        help.wrap(_S("A type-specific procedure compiles the file's contents into an efficient format ")
                  _S("for use by the runtime. A data-buffer is provided to HECL.\n\n"));
        help.wrapBold(_S("- Hash and Compress: "));
        help.wrap(_S("The data-buffer is hashed and compressed before being cached in the object database.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<pathspec>..."), _S("input file(s)"));
        help.beginWrap();
        help.wrap(_S("Specifies working file(s) containing production data to be cooked by HECL. ")
                  _S("Glob-strings may be specified (e.g. "));
        help.wrapBold(_S("*.blend"));
        help.wrap(_S(") to automatically cook all matching current-directory files in the project database. ")
                  _S("If no path specified, all files in the project database are cooked.\n"));
        help.endWrap();

        help.optionHead(_S("-r"), _S("recursion"));
        help.beginWrap();
        help.wrap(_S("Enables recursive file-matching for cooking entire directories of working files.\n"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("cook");}

    int run()
    {
        for (const HECL::ProjectPath& path : m_selectedItems)
        {
            int lineIdx = 0;
            m_useProj->cookPath(path,
            [&lineIdx](const HECL::SystemChar* message, const HECL::SystemChar* submessage,
                       int lidx, float factor)
            {
                ToolPrintProgress(message, submessage, lidx, factor, lineIdx);
            }, m_recursive);
        }
        return 0;
    }
};

#endif // CTOOL_COOK
