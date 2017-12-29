#ifndef CTOOL_SPEC
#define CTOOL_SPEC

#include "ToolBase.hpp"
#include <cstdio>
#include <map>

class ToolSpec final : public ToolBase
{
    enum Mode
    {
        MLIST = 0,
        MENABLE,
        MDISABLE
    } mode = MLIST;
public:
    ToolSpec(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (info.args.empty())
            return;

        if (!info.project)
            LogModule.report(logvisor::Fatal,
                             "hecl spec must be ran within a project directory");

        const auto& specs = info.project->getDataSpecs();
        hecl::SystemString firstArg = info.args.front();
        hecl::ToLower(firstArg);

        if (!firstArg.compare(_S("enable")))
            mode = MENABLE;
        else if (!firstArg.compare(_S("disable")))
            mode = MDISABLE;
        else
            return;

        if (info.args.size() < 2)
            LogModule.report(logvisor::Fatal, "Speclist argument required");

        auto it = info.args.begin();
        ++it;
        for (;it != info.args.end();
             ++it)
        {

            bool found = false;
            for (auto& spec : specs)
            {
                if (!it->compare(spec.spec.m_name))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                LogModule.report(logvisor::Fatal,
                                 _S("'%s' is not found in the dataspec registry"),
                                 it->c_str());
        }
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-spec - Configure target data options\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl spec [enable|disable] [<specname>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command configures the HECL project with the user's preferred target DataSpecs.\n\n")
                  _S("Providing enable/disable argument will bulk-set the enable status of the provided spec(s)")
                  _S("list. If enable/disable is not provided, a list of supported DataSpecs is printed.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<specname>..."), _S("DataSpec name(s)"));
        help.beginWrap();
        help.wrap(_S("Specifies platform-names to enable/disable"));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("spec");}

    int run()
    {
        if (!m_info.project)
        {
            for (const hecl::Database::DataSpecEntry* spec : hecl::Database::DATA_SPEC_REGISTRY)
            {
                if (XTERM_COLOR)
                    hecl::Printf(_S("" BOLD CYAN "%s" NORMAL "\n"), spec->m_name.data());
                else
                    hecl::Printf(_S("%s\n"), spec->m_name.data());
                hecl::Printf(_S("  %s\n"), spec->m_desc.data());
            }
            return 0;
        }

        const auto& specs = m_info.project->getDataSpecs();
        if (mode == MLIST)
        {
            for (auto& spec : specs)
            {
                if (XTERM_COLOR)
                    hecl::Printf(_S("" BOLD CYAN "%s" NORMAL ""), spec.spec.m_name.data());
                else
                    hecl::Printf(_S("%s"), spec.spec.m_name.data());
                if (spec.active)
                {
                    if (XTERM_COLOR)
                        hecl::Printf(_S(" " BOLD GREEN "[ENABLED]" NORMAL ""));
                    else
                        hecl::Printf(_S(" [ENABLED]"));
                }
                hecl::Printf(_S("\n  %s\n"), spec.spec.m_desc.data());
            }
            return 0;
        }

        std::vector<hecl::SystemString> opSpecs;
        auto it = m_info.args.begin();
        ++it;
        for (; it != m_info.args.end() ; ++it)
        {
            hecl::SystemString itName = *it;
            hecl::ToLower(itName);
            for (auto& spec : specs)
            {
                hecl::SystemString compName(spec.spec.m_name);
                hecl::ToLower(compName);
                if (!itName.compare(compName))
                {
                    opSpecs.emplace_back(spec.spec.m_name);
                    break;
                }
            }
        }

        if (opSpecs.size())
        {
            if (mode == MENABLE)
                m_info.project->enableDataSpecs(opSpecs);
            else if (mode == MDISABLE)
                m_info.project->disableDataSpecs(opSpecs);
        }

        return 0;
    }
};

#endif // CTOOL_SPEC
