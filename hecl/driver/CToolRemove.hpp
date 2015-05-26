#ifndef CTOOL_REMOVE
#define CTOOL_REMOVE

#include "CToolBase.hpp"
#include <stdio.h>

class CToolRemove final : public CToolBase
{
public:
    CToolRemove(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolRemove()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-rm\n");
        help.wrap("hecl-remove - Remove working files from the HECL index\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl remove [-r] [<pathspec>...]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command removes a file, directory, or glob-pattern of files from the project database. "
                  "Once a file is removed, any cooked cache objects are deleted automatically. ");
        help.wrapBold("The working file itself is not deleted from the filesystem.\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<pathspec>...", "input file(s)");
        help.beginWrap();
        help.wrap("Working file(s) to be removed from the project database. "
                  "Glob-strings may be specified (e.g. ");
        help.wrapBold("*.blend");
        help.wrap(") to automatically remove all matching files from the database.\n");
        help.endWrap();

        help.optionHead("-r", "recursion");
        help.beginWrap();
        help.wrap("Enables recursive file-matching for removing entire directories of working files.\n");
        help.endWrap();
    }

    std::string toolName() const {return "remove";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_REMOVE
