#ifndef CTOOL_CLEAN
#define CTOOL_CLEAN

#include "CToolBase.hpp"
#include <stdio.h>

class CToolClean final : public CToolBase
{
public:
    CToolClean(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolClean()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-clean - Delete cached cooked objects referenced via working files\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl clean [-ri] [<pathspec>...]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command performs an immediate deletion of cooked objects cached "
                  "within the project database. It may operate on a subset of objects or the "
                  "entire project.\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<pathspec>...", "clean path(s)");
        help.beginWrap();
        help.wrap("When one or more paths are specified in the command, the clean process will "
                  "restrict object deletion to only the working file(s) specified. If ");
        help.wrapBold("-r");
        help.wrap(" is also specifed, directories may be provided as well. If no path(s) specified, "
                  "the entire project is cleaned.\n");
        help.endWrap();

        help.optionHead("-r", "recursion");
        help.beginWrap();
        help.wrap("Enables recursive file-matching for cleaning entire directories of working files.\n");
        help.endWrap();

        help.optionHead("-i", "follow implicit links");
        help.beginWrap();
        help.wrap("Enables implicit object traversal and cleaning. This is only useful if one or more paths "
                  "are specified. For objects supporting implicit-gathering, this will query those "
                  "objects for their current implicit links and ensure the linked-objects are cleaned "
                  "as well.\n");
        help.endWrap();
    }

    std::string toolName() const {return "clean";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_CLEAN
