#ifndef CTOOL_INIT
#define CTOOL_INIT

#include "ToolBase.hpp"
#include <cstdio>

class ToolInit final : public ToolBase
{
    const hecl::SystemString* m_dir = NULL;
public:
    ToolInit(const ToolPassInfo& info)
    : ToolBase(info)
    {
        hecl::Sstat theStat;
        const hecl::SystemString* dir;
        if (info.args.size())
            dir = &info.args.front();
        else
            dir = &info.cwd;

        if (hecl::Stat(dir->c_str(), &theStat))
        {
            hecl::MakeDir(dir->c_str());
            if (hecl::Stat(dir->c_str(), &theStat))
            {
                LogModule.report(logvisor::Fatal, _S("unable to stat '%s'"), dir->c_str());
                return;
            }
        }
        if (!S_ISDIR(theStat.st_mode))
        {
            LogModule.report(logvisor::Fatal, _S("'%s' is not a directory"), dir->c_str());
            return;
        }

        hecl::SystemString testPath = *dir + _S("/.hecl/beacon");
        if (!hecl::Stat(testPath.c_str(), &theStat))
        {
            LogModule.report(logvisor::Fatal, _S("project already exists at '%s'"), dir->c_str());
            return;
        }

        m_dir = dir;
    }

    int run()
    {
        if (!m_dir)
            return 1;
        size_t ErrorRef = logvisor::ErrorCount;
        hecl::Database::Project proj((hecl::ProjectRootPath(*m_dir)));
        if (logvisor::ErrorCount > ErrorRef)
            return 1;
        LogModule.report(logvisor::Info, _S("initialized project at '%s/.hecl'"), m_dir->c_str());
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
        help.wrap(_S(" directory within the selected directory with an initialized database index. ")
                  _S("This constitutes an empty HECL project, ready for making stuff!!\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<dir>"), _S("group directory path"));
        help.beginWrap();
        help.wrap(_S("Directory to create new project database in. If not specified, current directory is used.\n"));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("init");}
};

#endif // CTOOL_INIT
