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
            throw HECL::Exception(_S("unable to stat '") + *dir + _S("'"));
            return;
        }
        if (!S_ISDIR(theStat.st_mode))
        {
            throw HECL::Exception(_S("'") + *dir + _S("' is not a directory"));
            return;
        }

        HECL::SystemString testPath = *dir + _S("/.hecl/index");
        if (!HECL::Stat(testPath.c_str(), &theStat))
        {
            throw HECL::Exception(_S("project already exists at '") + *dir + _S("'"));
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
            HECL::Database::Project(HECL::ProjectRootPath(*m_dir));
        }
        catch (HECL::Exception& e)
        {
            HECL::FPrintf(stderr, _S("unable to init project: '%s'\n"), e.swhat());
            return -1;
        }
        HECL::Printf(_S("initialized project at '%s/.hecl'\n"), m_dir->c_str());
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
