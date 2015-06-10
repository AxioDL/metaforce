#ifndef CTOOL_REMOVE
#define CTOOL_REMOVE

#include "ToolBase.hpp"
#include <stdio.h>

class ToolRemove final : public ToolBase
{
public:
    ToolRemove(const ToolPassInfo& info)
    : ToolBase(info)
    {
    }

    ~ToolRemove()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-rm\n"));
        help.wrap(_S("hecl-remove - Remove working files from the HECL index\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl remove [-r] [<pathspec>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command removes a file, directory, or glob-pattern of files from the project database. "
                  "Once a file is removed, any cooked cache objects are deleted automatically. "));
        help.wrapBold(_S("The working file itself is not deleted from the filesystem.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<pathspec>..."), _S("input file(s)"));
        help.beginWrap();
        help.wrap(_S("Working file(s) to be removed from the project database. "
                  "Glob-strings may be specified (e.g. "));
        help.wrapBold(_S("*.blend"));
        help.wrap(_S(") to automatically remove all matching files from the database.\n"));
        help.endWrap();

        help.optionHead(_S("-r"), _S("recursion"));
        help.beginWrap();
        help.wrap(_S("Enables recursive file-matching for removing entire directories of working files.\n"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("remove");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_REMOVE
