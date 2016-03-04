#ifndef CTOOL_ADD
#define CTOOL_ADD

#include "ToolBase.hpp"
#include <stdio.h>

class ToolAdd final : public ToolBase
{
public:
    ToolAdd(const ToolPassInfo& info)
    : ToolBase(info)
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-add - Add working files to the HECL index\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl add [<pathspec>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command stages a file or glob-pattern of files within the project database ")
                  _S("for inclusion in the "));
        help.wrapBold(_S("hecl cook"));
        help.wrap(_S(" process.\n\n")
                  _S("Files added in this manner automatically become 'explicit' database ")
                  _S("objects. 'Explicit objects' will not be removed in housekeeping tasks automatically ")
                  _S("performed by HECL's library functions, unless the user (re)moves the file ")
                  _S("using the filesystem.\n\n")
                  _S("For details on explicit vs. implicit objects, view the "));
        help.wrapBold(_S("hecl cook"));
        help.wrap(_S(" documentation.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<pathspec>..."), _S("input file(s)"));
        help.beginWrap();
        help.wrap(_S("Working file(s) containing production data to be cooked by HECL. ")
                  _S("Glob-strings may be specified (e.g. "));
        help.wrapBold(_S("*.blend"));
        help.wrap(_S(") to automatically add all matching files to the database.\n"));
        help.endWrap();
    }

    hecl::SystemString toolName() const {return _S("add");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_ADD
