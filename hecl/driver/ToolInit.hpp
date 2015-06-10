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
        if (info.args.size())
        {

        }
        m_dir = &info.args[0];
    }

    ~ToolInit()
    {
    }

    int run()
    {
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
