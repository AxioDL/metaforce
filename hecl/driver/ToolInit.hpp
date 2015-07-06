#ifndef CTOOL_INIT
#define CTOOL_INIT

#include "ToolBase.hpp"
#include <stdio.h>

class ToolInit final : public ToolBase
{
    const HECL::SystemString* m_dir = NULL;
public:
    ToolInit(const ToolPassInfo& info)
    : ToolBase(info)
    {
        struct stat theStat;
        const HECL::SystemString* dir;
        if (info.args.size())
            dir = &info.args[0];
        else
            dir = &info.cwd;

        if (HECL::Stat(dir->c_str(), &theStat))
        {
            LogModule.report(LogVisor::FatalError, _S("unable to stat '%s'"), dir->c_str());
            return;
        }
        if (!S_ISDIR(theStat.st_mode))
        {
            LogModule.report(LogVisor::FatalError, _S("'%s' is not a directory"), dir->c_str());
            return;
        }

        HECL::SystemString testPath = *dir + _S("/.hecl/beacon");
        if (!HECL::Stat(testPath.c_str(), &theStat))
        {
            LogModule.report(LogVisor::FatalError, _S("project already exists at '%s'"), dir->c_str());
            return;
        }

        m_dir = dir;
    }

    int run()
    {
        if (!m_dir)
            return -1;
        try
        {
            HECL::Database::Project proj((HECL::ProjectRootPath(*m_dir)));
            proj.enableDataSpecs({_S("hecl-little")});
        }
        catch (std::exception& e)
        {
            LogModule.report(LogVisor::Error, "unable to init project");
            return -1;
        }
        LogModule.report(LogVisor::Info, _S("initialized project at '%s/.hecl'"), m_dir->c_str());
        return 0;
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-init - Initialize a brand-new project database\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl init [<dir>]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("Creates a "));
        help.wrapBold(_S(".hecl"));
        help.wrap(_S(" directory within the selected directory with an initialized database index. "
                  "This constitutes an empty HECL project, ready for making stuff!!\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<dir>"), _S("group directory path"));
        help.beginWrap();
        help.wrap(_S("Directory to create new project database in. If not specified, current directory is used.\n"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("init");}
};

#endif // CTOOL_INIT
