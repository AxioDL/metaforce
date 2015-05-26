#ifndef CTOOL_INIT
#define CTOOL_INIT

#include "CToolBase.hpp"
#include <stdio.h>

class CToolInit final : public CToolBase
{
public:
    CToolInit(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolInit()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-init - Initialize a brand-new project database\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl init [<dir>]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("Creates a ");
        help.wrapBold(".hecl");
        help.wrap(" directory within the selected directory with an initialized database index. "
                  "This constitutes an empty HECL project, ready for making stuff!!\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<dir>", "group directory path");
        help.beginWrap();
        help.wrap("Directory to create new project database in. If not specified, current directory is used.\n");
        help.endWrap();
    }

    std::string toolName() const {return "init";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_INIT
