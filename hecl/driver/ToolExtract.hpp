#ifndef CTOOL_EXTRACT
#define CTOOL_EXTRACT

#include "ToolBase.hpp"
#include <stdio.h>
#include <conio.h>

class ToolExtract final : public ToolBase
{
    HECL::Database::IDataSpec::ExtractPassInfo m_einfo;
    struct SpecExtractPass
    {
        const HECL::Database::DataSpecEntry* m_entry;
        std::unique_ptr<HECL::Database::IDataSpec> m_instance;
        SpecExtractPass(const HECL::Database::DataSpecEntry* entry, HECL::Database::IDataSpec* instance)
        : m_entry(entry), m_instance(instance) {}
        SpecExtractPass(const SpecExtractPass& other) = delete;
        SpecExtractPass(SpecExtractPass&& other) = default;
    };
    std::vector<SpecExtractPass> m_specPasses;
    std::vector<HECL::Database::IDataSpec::ExtractReport> m_reps;
    std::unique_ptr<HECL::Database::Project> m_fallbackProj;
    HECL::Database::Project* m_useProj;
public:
    ToolExtract(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (!m_info.args.size())
            LogModule.report(LogVisor::FatalError, "hecl extract needs a source path as its first argument");

        if (!info.project)
        {
            /* Get name from input file and init project there */
            HECL::SystemString baseFile = info.args[0];
            size_t slashPos = baseFile.rfind(_S('/'));
            if (slashPos == HECL::SystemString::npos)
                slashPos = baseFile.rfind(_S('\\'));
            if (slashPos != HECL::SystemString::npos)
                baseFile.assign(baseFile.begin() + slashPos + 1, baseFile.end());
            size_t dotPos = baseFile.rfind(_S('.'));
            if (dotPos != HECL::SystemString::npos)
                baseFile.assign(baseFile.begin(), baseFile.begin() + dotPos);

            if (baseFile.empty())
                LogModule.report(LogVisor::FatalError, "hecl extract must be ran within a project directory");

            size_t ErrorRef = LogVisor::ErrorCount;
            HECL::SystemString rootDir = info.cwd + _S('/') + baseFile;
            HECL::ProjectRootPath newProjRoot(rootDir);
            newProjRoot.makeDir();
            m_fallbackProj.reset(new HECL::Database::Project(newProjRoot));
            if (LogVisor::ErrorCount > ErrorRef)
                LogModule.report(LogVisor::FatalError, "unable to init project at '%s'", rootDir.c_str());
            LogModule.report(LogVisor::Info, _S("initialized project at '%s/.hecl'"), rootDir.c_str());
            m_useProj = m_fallbackProj.get();
        }
        else
            m_useProj = info.project;

        m_einfo.srcpath = m_info.args[0];
        m_einfo.extractArgs.reserve(info.args.size() - 1);
        m_einfo.force = info.force;
        for (std::vector<HECL::SystemString>::const_iterator it=info.args.begin() + 1;
             it != info.args.end();
             ++it)
            m_einfo.extractArgs.push_back(*it);

        for (const HECL::Database::DataSpecEntry* entry : HECL::Database::DATA_SPEC_REGISTRY)
        {
            HECL::Database::IDataSpec* ds = entry->m_factory(*m_useProj, HECL::Database::TOOL_EXTRACT);
            if (ds)
            {
                if (ds->canExtract(m_einfo, m_reps))
                    m_specPasses.emplace_back(entry, ds);
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

    HECL::SystemString toolName() const {return _S("extract");}

    static void _recursivePrint(int level, HECL::Database::IDataSpec::ExtractReport& rep)
    {
        for (int l=0 ; l<level ; ++l)
            HECL::Printf(_S("  "));
#if _WIN32
        HECL::Printf(_S("%s"), rep.name.c_str());
#else
        if (XTERM_COLOR)
            HECL::Printf(_S("" BOLD "%s" NORMAL ""), rep.name.c_str());
        else
            HECL::Printf(_S("%s"), rep.name.c_str());
#endif
        if (rep.desc.size())
            HECL::Printf(_S(" [%s]"), rep.desc.c_str());
        HECL::Printf(_S("\n"));
        for (HECL::Database::IDataSpec::ExtractReport& child : rep.childOpts)
            _recursivePrint(level + 1, child);
    }

    int run()
    {
        if (m_specPasses.empty())
        {
#if _WIN32
            HECL::Printf(_S("NOTHING TO EXTRACT\n"));
#else
            if (XTERM_COLOR)
                HECL::Printf(_S("" RED BOLD "NOTHING TO EXTRACT" NORMAL "\n"));
            else
                HECL::Printf(_S("NOTHING TO EXTRACT\n"));
#endif
            return -1;
        }

#if _WIN32
        HECL::Printf(_S("ABOUT TO EXTRACT:\n"));
#else
        if (XTERM_COLOR)
            HECL::Printf(_S("" GREEN BOLD "ABOUT TO EXTRACT:" NORMAL "\n"));
        else
            HECL::Printf(_S("ABOUT TO EXTRACT:\n"));
#endif

        for (HECL::Database::IDataSpec::ExtractReport& rep : m_reps)
        {
            _recursivePrint(0, rep);
            HECL::Printf(_S("\n"));
        }

#if _WIN32
        HECL::Printf(_S("\nContinue? (Y/n) "));
#else
        if (XTERM_COLOR)
            HECL::Printf(_S("\n" BLUE BOLD "Continue?" NORMAL " (Y/N) "));
        else
            HECL::Printf(_S("\nContinue? (Y/N) "));
#endif

        int ch;
        while ((ch = getch()))
        {
            if (ch == 'n' || ch == 'N')
                return 0;
            if (ch == 'y' || ch == 'Y' || ch == 13)
                break;
        }
        HECL::Printf(_S("\n"));

        for (SpecExtractPass& ds : m_specPasses)
        {
#if _WIN32
            HECL::Printf(_S("Using DataSpec %s:\n"), ds.m_entry->m_name);
#else
            if (XTERM_COLOR)
                HECL::Printf(_S("" MAGENTA BOLD "Using DataSpec %s:" NORMAL "\n"), ds.m_entry->m_name);
            else
                HECL::Printf(_S("Using DataSpec %s:\n"), ds.m_entry->m_name);
#endif

            int lineIdx = 0;
            int prevIFactor = -1;
            ds.m_instance->doExtract(m_einfo,
                                     [&lineIdx, &prevIFactor](const HECL::SystemChar* message, int lidx, float factor)
            {
                int iFactor = factor * 100.0;
                if (iFactor == prevIFactor)
                    return;
                prevIFactor = iFactor;
#ifndef _WIN32
                if (XTERM_COLOR)
                    HECL::Printf(_S("" HIDE_CURSOR ""));
#endif

                if (lidx > lineIdx)
                {
                    HECL::Printf(_S("\n  "));
                    lineIdx = lidx;
                }
                else
                    HECL::Printf(_S("  "));

                int width = HECL::ConsoleWidth();
                int half = width / 2 - 2;

                size_t messageLen = HECL::StrLen(message);
                if (messageLen > half)
                    HECL::Printf(_S("%.*s... "), half-3, message);
                else
                {
                    HECL::Printf(_S("%s"), message);
                    for (int i=half-messageLen ; i>=0 ; --i)
                        HECL::Printf(_S(" "));
                }

#ifndef _WIN32
                if (XTERM_COLOR)
                {
                    size_t blocks = half - 7;
                    size_t filled = blocks * factor;
                    size_t rem = blocks - filled;
                    HECL::Printf(_S("" BOLD "%3d%% ["), iFactor);
                    for (int b=0 ; b<filled ; ++b)
                        HECL::Printf(_S("#"), message);
                    for (int b=0 ; b<rem ; ++b)
                        HECL::Printf(_S("-"), message);
                    HECL::Printf(_S("]" NORMAL ""));
                }
                else
                {
#endif
                    size_t blocks = half - 7;
                    size_t filled = blocks * factor;
                    size_t rem = blocks - filled;
                    HECL::Printf(_S("%3d%% ["), iFactor);
                    for (int b=0 ; b<filled ; ++b)
                        HECL::Printf(_S("#"), message);
                    for (int b=0 ; b<rem ; ++b)
                        HECL::Printf(_S("-"), message);
                    HECL::Printf(_S("]"));
#ifndef _WIN32
                }
#endif

                HECL::Printf(_S("\r"));
#ifndef _WIN32
                if (XTERM_COLOR)
                    HECL::Printf(_S("" SHOW_CURSOR ""));
#endif
                fflush(stdout);
            });
            HECL::Printf(_S("\n\n"));
        }

        return 0;
    }
};

#endif // CTOOL_EXTRACT
