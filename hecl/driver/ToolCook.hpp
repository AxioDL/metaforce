#ifndef CTOOL_COOK
#define CTOOL_COOK

#include "ToolBase.hpp"
#include <cstdio>
#include "hecl/ClientProcess.hpp"

class ToolCook final : public ToolBase
{
    std::vector<hecl::ProjectPath> m_selectedItems;
    std::unique_ptr<hecl::Database::Project> m_fallbackProj;
    hecl::Database::Project* m_useProj;
    const hecl::Database::DataSpecEntry* m_spec = nullptr;
    bool m_recursive = false;
    bool m_fast = false;
public:
    ToolCook(const ToolPassInfo& info)
    : ToolBase(info), m_useProj(info.project)
    {
        /* Check for recursive flag */
        for (hecl::SystemChar arg : info.flags)
            if (arg == _S('r'))
                m_recursive = true;

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
                                         _S("hecl cook can only process multiple items in the same project; ")
                                         _S("'%s' and '%s' are different projects"),
                                         m_fallbackProj->getProjectRootPath().getAbsolutePath().data(),
                                         root.getAbsolutePath().data());
                    m_selectedItems.emplace_back(*m_useProj, subPath);
                }
            }
        }
        if (!m_useProj)
            LogModule.report(logvisor::Fatal,
                             "hecl cook must be ran within a project directory or "
                             "provided a path within a project");

        /* Default case: recursive at root */
        if (m_selectedItems.empty())
        {
            m_selectedItems.reserve(1);
            m_selectedItems.push_back({hecl::ProjectPath(*m_useProj, _S(""))});
            m_recursive = true;
        }
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-cook - Cook objects within the project database\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl cook [-rf] [--fast] [--spec=<spec>] [<pathspec>...]\n"));
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
        help.optionHead(_S("-f"), _S("force"));
        help.beginWrap();
        help.wrap(_S("Forces cooking of all matched files, ignoring timestamp differences.\n"));
        help.endWrap();
        help.optionHead(_S("--fast"), _S("fast cook"));
        help.beginWrap();
        help.wrap(_S("Performs draft-optimization cooking for supported data types.\n"));
        help.endWrap();

        help.optionHead(_S("--spec=<spec>"), _S("data specification"));
        help.beginWrap();
        help.wrap(_S("Specifies a DataSpec to use when cooking. ")
                  _S("This build of hecl supports the following values of <spec>:\n"));
        for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY)
        {
            if (!spec->m_factory)
                continue;
            help.wrap(_S("  "));
            help.wrapBold(spec->m_name.data());
            help.wrap(_S("\n"));
        }
    }

    hecl::SystemString toolName() const {return _S("cook");}

    int run()
    {
        hecl::MultiProgressPrinter printer(true);
        hecl::ClientProcess cp(&printer, m_info.verbosityLevel);
        for (int i=0 ; i<m_spec->m_numCookPasses ; ++i)
            for (const hecl::ProjectPath& path : m_selectedItems)
                m_useProj->cookPath(path, printer, m_recursive, m_info.force, m_fast, m_spec, &cp, i);
        cp.waitUntilComplete();
        return 0;
    }
};

#endif // CTOOL_COOK
