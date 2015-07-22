#ifndef CTOOL_GROUP
#define CTOOL_GROUP

#include "ToolBase.hpp"
#include <stdio.h>

class ToolGroup final : public ToolBase
{
public:
    ToolGroup(const ToolPassInfo& info)
    : ToolBase(info)
    {
        if (!info.project)
            LogModule.report(LogVisor::FatalError, "hecl group must be ran within a project directory");
    }

    ~ToolGroup()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-group - Fork a project directory as an explicit group\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl group [-D] <dir>\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command turns a nested subdirectory of the project into a HECL group. ")
                  _S("Groups play an important role in the resulting structure of the packaged ")
                  _S("database. All objects in HECL belong to a group of some sort since the runtime ")
                  _S("only provides loading functions for groups. Ungrouped ")
                  _S("objects in the project root are individually added to 'loose groups'.\n\n With "));
        help.wrapBold(_S("hecl group"));
        help.wrap(_S(", explicit groups may be defined (e.g. a stage, level, area, loadable segment). "));
        help.wrap(_S("Groups are defined by filesystem directories relative to the project root ")
                  _S("and may be loaded within the runtime using the relative path as a lookup-string. ")
                  _S("Sub-directories that aren't explicitly made into a group inherit the group-status ")
                  _S("of the parent directory.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<dir>"), _S("group directory path"));
        help.beginWrap();
        help.wrap(_S("Directory to fork as an explicit group\n"));
        help.endWrap();

        help.optionHead(_S("-D"), _S("delete group"));
        help.beginWrap();
        help.wrap(_S("Remove's directory's status as an explicit group; restoring its inheritance ")
                  _S("from the parent directory.\n"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("group");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_GROUP
