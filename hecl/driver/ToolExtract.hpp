#ifndef CTOOL_EXTRACT
#define CTOOL_EXTRACT

#include "ToolBase.hpp"
#include <stdio.h>

class ToolExtract final : public ToolBase
{
    HECL::Database::IDataSpec::ExtractPassInfo m_einfo;
    std::vector<std::unique_ptr<HECL::Database::IDataSpec>> m_dataSpecs;
    std::vector<HECL::Database::IDataSpec::ExtractReport> m_reps;
public:
    ToolExtract(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (!info.project)
            LogModule.report(LogVisor::FatalError, "hecl extract must be ran within a project directory");
        if (!m_info.args.size())
            LogModule.report(LogVisor::FatalError, "hecl extract needs a source path as its first argument");

        m_einfo.srcpath = m_info.args[0];
        m_einfo.extractArgs.reserve(info.args.size() - 1);
        for (std::vector<HECL::SystemString>::const_iterator it=info.args.begin() + 1;
             it != info.args.end();
             ++it)
            m_einfo.extractArgs.push_back(*it);

        for (const HECL::Database::DataSpecEntry* entry : HECL::Database::DATA_SPEC_REGISTRY)
        {
            HECL::Database::IDataSpec* ds = entry->m_factory(HECL::Database::TOOL_EXTRACT);
            if (ds)
            {
                if (ds->canExtract(*m_info.project, m_einfo, m_reps))
                    m_dataSpecs.emplace_back(ds);
                else
                    delete ds;
            }
        }
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-extract - Extract objects from supported package/image formats\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl extract <packagefile> [<subnode>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command recursively extracts all or part of a dataspec-supported "
                     "package format. Each object is decoded to a working format and added to the project.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<packagefile>[/<subnode>...]"), _S("input file"));
        help.beginWrap();
        help.wrap(_S("Specifies the package file or disc image to source data from. "
                     "An optional subnode specifies a named hierarchical-node specific "
                     "to the game architecture (levels/areas)."));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("extract");}

    static void _recursivePrint(int level, HECL::Database::IDataSpec::ExtractReport& rep)
    {
        for (int l=0 ; l<level ; ++l)
            HECL::Printf(_S("  "));
        if (XTERM_COLOR)
            HECL::Printf(_S("" BOLD "%s" NORMAL ""), rep.name.c_str());
        else
            HECL::Printf(_S("%s"), rep.name.c_str());
        if (rep.desc.size())
            HECL::Printf(_S(" [%s]"), rep.desc.c_str());
        HECL::Printf(_S("\n"));
        for (HECL::Database::IDataSpec::ExtractReport& child : rep.childOpts)
            _recursivePrint(level + 1, child);
    }

    int run()
    {
        if (m_dataSpecs.empty())
        {
            if (XTERM_COLOR)
                HECL::Printf(_S("" RED BOLD "NOTHING TO EXTRACT" NORMAL "\n"));
            else
                HECL::Printf(_S("NOTHING TO EXTRACT\n"));
            return -1;
        }

        if (XTERM_COLOR)
            HECL::Printf(_S("" GREEN BOLD "ABOUT TO EXTRACT:" NORMAL "\n"));
        else
            HECL::Printf(_S("ABOUT TO EXTRACT:\n"));

        for (HECL::Database::IDataSpec::ExtractReport& rep : m_reps)
        {
            _recursivePrint(0, rep);
            HECL::Printf(_S("\n"));
        }

        if (XTERM_COLOR)
            HECL::Printf(_S("\n" BLUE BOLD "Continue?" NORMAL " (Y/N) "));
        else
            HECL::Printf(_S("\nContinue? (Y/N) "));

        int ch;
        while ((ch = getchar()))
        {
            if (ch == 'n' || ch == 'N')
                return 0;
            if (ch == 'y' || ch == 'Y')
                break;
        }

        for (std::unique_ptr<HECL::Database::IDataSpec>& ds : m_dataSpecs)
            ds->doExtract(*m_info.project, m_einfo);

        return 0;
    }
};

#endif // CTOOL_EXTRACT
