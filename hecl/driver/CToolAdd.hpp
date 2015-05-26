#ifndef CTOOL_ADD
#define CTOOL_ADD

#include "CToolBase.hpp"
#include <stdio.h>

class CToolAdd final : public CToolBase
{
public:
    CToolAdd(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolAdd()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-add - Add working files to the HECL index\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl add [<pathspec>...]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command stages a file or glob-pattern of files within the project database "
                  "for inclusion in the ");
        help.wrapBold("hecl cook");
        help.wrap(" process.\n\n"
                  "Files added in this manner automatically become 'explicit' database "
                  "objects. 'Explicit objects' will not be removed in housekeeping tasks automatically "
                  "performed by HECL's library functions, unless the user (re)moves the file "
                  "using the filesystem.\n\n"
                  "For details on explicit vs. implicit objects, view the ");
        help.wrapBold("hecl cook");
        help.wrap(" documentation.\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<pathspec>...", "input file(s)");
        help.beginWrap();
        help.wrap("Working file(s) containing production data to be cooked by HECL. "
                  "Glob-strings may be specified (e.g. ");
        help.wrapBold("*.blend");
        help.wrap(") to automatically add all matching files to the database.\n");
        help.endWrap();
    }

    std::string toolName() const {return "add";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_ADD
