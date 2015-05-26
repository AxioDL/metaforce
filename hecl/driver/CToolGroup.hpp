#ifndef CTOOL_GROUP
#define CTOOL_GROUP

#include "CToolBase.hpp"
#include <stdio.h>

class CToolGroup final : public CToolBase
{
public:
    CToolGroup(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolGroup()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-group - Fork a project directory as an explicit group\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl group [-D] <dir>\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command turns a nested subdirectory of the project into a HECL group. "
                  "Groups play an important role in the resulting structure of the packaged "
                  "database. All objects in HECL belong to a group of some sort since the runtime "
                  "only provides loading functions for groups. Ungrouped "
                  "objects in the project root are individually added to 'loose groups'.\n\n With ");
        help.wrapBold("hecl group");
        help.wrap(", explicit groups may be defined (e.g. a stage, level, area, loadable segment). ");
        help.wrap("Groups are defined by filesystem directories relative to the project root "
                  "and may be loaded within the runtime using the relative path as a lookup-string. "
                  "Sub-directories that aren't explicitly made into a group inherit the group-status "
                  "of the parent directory.\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<dir>", "group directory path");
        help.beginWrap();
        help.wrap("Directory to fork as an explicit group\n");
        help.endWrap();

        help.optionHead("-D", "delete group");
        help.beginWrap();
        help.wrap("Remove's directory's status as an explicit group; restoring its inheritance "
                  "from the parent directory.\n");
        help.endWrap();
    }

    std::string toolName() const {return "group";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_GROUP
