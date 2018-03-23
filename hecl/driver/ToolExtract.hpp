#ifndef CTOOL_EXTRACT
#define CTOOL_EXTRACT

#include "ToolBase.hpp"
#include <cstdio>

#if _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

#include "hecl/MultiProgressPrinter.hpp"

class ToolExtract final : public ToolBase
{
    hecl::Database::IDataSpec::ExtractPassInfo m_einfo;
    struct SpecExtractPass
    {
        const hecl::Database::DataSpecEntry* m_entry;
        std::unique_ptr<hecl::Database::IDataSpec> m_instance;
        SpecExtractPass(const hecl::Database::DataSpecEntry* entry, hecl::Database::IDataSpec* instance)
        : m_entry(entry), m_instance(instance) {}
        SpecExtractPass(const SpecExtractPass& other) = delete;
        SpecExtractPass(SpecExtractPass&& other) = default;
    };
    std::vector<SpecExtractPass> m_specPasses;
    std::vector<hecl::Database::IDataSpec::ExtractReport> m_reps;
    std::unique_ptr<hecl::Database::Project> m_fallbackProj;
    hecl::Database::Project* m_useProj = nullptr;
public:
    ToolExtract(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (!m_info.args.size())
            LogModule.report(logvisor::Fatal, "hecl extract needs a source path as its first argument");

        if (!info.project)
        {
            hecl::SystemString rootDir;

            if (info.output.empty())
            {
                /* Get name from input file and init project there */
                hecl::SystemString baseFile = info.args.front();
                size_t slashPos = baseFile.rfind(_S('/'));
                if (slashPos == hecl::SystemString::npos)
                    slashPos = baseFile.rfind(_S('\\'));
                if (slashPos != hecl::SystemString::npos)
                    baseFile.assign(baseFile.begin() + slashPos + 1, baseFile.end());
                size_t dotPos = baseFile.rfind(_S('.'));
                if (dotPos != hecl::SystemString::npos)
                    baseFile.assign(baseFile.begin(), baseFile.begin() + dotPos);

                if (baseFile.empty())
                    LogModule.report(logvisor::Fatal, "hecl extract must be ran within a project directory");

                rootDir = info.cwd + baseFile;
            }
            else
            {
                if (hecl::PathRelative(info.output.c_str()))
                    rootDir = info.cwd + info.output;
                else
                    rootDir = info.output;
            }

            size_t ErrorRef = logvisor::ErrorCount;
            hecl::ProjectRootPath newProjRoot(rootDir);
            newProjRoot.makeDir();
            m_fallbackProj.reset(new hecl::Database::Project(newProjRoot));
            if (logvisor::ErrorCount > ErrorRef)
                LogModule.report(logvisor::Fatal, "unable to init project at '%s'", rootDir.c_str());
            LogModule.report(logvisor::Info, _S("initialized project at '%s/.hecl'"), rootDir.c_str());
            m_useProj = m_fallbackProj.get();
        }
        else
            m_useProj = info.project;

        m_einfo.srcpath = m_info.args.front();
        m_einfo.force = info.force;
        m_einfo.extractArgs.reserve(info.args.size());
        auto it=info.args.cbegin();
        ++it;
        for (; it != info.args.cend(); ++it)
            m_einfo.extractArgs.push_back(*it);

        m_specPasses.reserve(hecl::Database::DATA_SPEC_REGISTRY.size());
        for (const hecl::Database::DataSpecEntry* entry : hecl::Database::DATA_SPEC_REGISTRY)
        {
            if (entry->m_factory)
            {
                hecl::Database::IDataSpec* ds = entry->m_factory(*m_useProj, hecl::Database::DataSpecTool::Extract);
                if (ds)
                {
                    if (ds->canExtract(m_einfo, m_reps))
                        m_specPasses.emplace_back(entry, ds);
                    else
                        delete ds;
                }
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
        help.wrap(_S("This command recursively extracts all or part of a dataspec-supported ")
                  _S("package format. Each object is decoded to a working format and added to the project.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<packagefile>[/<subnode>...]"), _S("input file"));
        help.beginWrap();
        help.wrap(_S("Specifies the package file or disc image to source data from. ")
                  _S("An optional subnode specifies a named hierarchical-node specific ")
                  _S("to the game architecture (levels/areas)."));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("extract");}

    static void _recursivePrint(int level, hecl::Database::IDataSpec::ExtractReport& rep)
    {
        for (int l=0 ; l<level ; ++l)
            hecl::Printf(_S("  "));
        if (XTERM_COLOR)
            hecl::Printf(_S("" BOLD "%s" NORMAL ""), rep.name.c_str());
        else
            hecl::Printf(_S("%s"), rep.name.c_str());

        if (rep.desc.size())
            hecl::Printf(_S(" [%s]"), rep.desc.c_str());
        hecl::Printf(_S("\n"));
        for (hecl::Database::IDataSpec::ExtractReport& child : rep.childOpts)
            _recursivePrint(level + 1, child);
    }

    int run()
    {
        if (m_specPasses.empty())
        {
            if (XTERM_COLOR)
                hecl::Printf(_S("" RED BOLD "NOTHING TO EXTRACT" NORMAL "\n"));
            else
                hecl::Printf(_S("NOTHING TO EXTRACT\n"));
            return -1;
        }

        if (XTERM_COLOR)
            hecl::Printf(_S("" GREEN BOLD "ABOUT TO EXTRACT:" NORMAL "\n"));
        else
            hecl::Printf(_S("ABOUT TO EXTRACT:\n"));

        for (hecl::Database::IDataSpec::ExtractReport& rep : m_reps)
        {
            _recursivePrint(0, rep);
            hecl::Printf(_S("\n"));
        }
        fflush(stdout);

        if (continuePrompt())
        {
            for (SpecExtractPass& ds : m_specPasses)
            {
                if (XTERM_COLOR)
                    hecl::Printf(_S("" MAGENTA BOLD "Using DataSpec %s:" NORMAL "\n"), ds.m_entry->m_name.data());
                else
                    hecl::Printf(_S("Using DataSpec %s:\n"), ds.m_entry->m_name.data());

                ds.m_instance->doExtract(m_einfo, {true});
                hecl::Printf(_S("\n\n"));
            }
        }

        return 0;
    }
};

#endif // CTOOL_EXTRACT
