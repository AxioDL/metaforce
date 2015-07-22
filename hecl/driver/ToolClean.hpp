#ifndef CTOOL_CLEAN
#define CTOOL_CLEAN

#include "ToolBase.hpp"
#include <stdio.h>

class ToolClean final : public ToolBase
{
public:
    ToolClean(const ToolPassInfo& info)
    : ToolBase(info)
    {
    }

    ~ToolClean()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-clean - Delete cached cooked objects referenced via working files\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl clean [-ri] [<pathspec>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command performs an immediate deletion of cooked objects cached ")
                  _S("within the project database. It may operate on a subset of objects or the ")
                  _S("entire project.\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<pathspec>..."), _S("clean path(s)"));
        help.beginWrap();
        help.wrap(_S("When one or more paths are specified in the command, the clean process will ")
                  _S("restrict object deletion to only the working file(s) specified. If "));
        help.wrapBold(_S("-r"));
        help.wrap(_S(" is also specifed, directories may be provided as well. If no path(s) specified, ")
                  _S("the entire project is cleaned.\n"));
        help.endWrap();

        help.optionHead(_S("-r"), _S("recursion"));
        help.beginWrap();
        help.wrap(_S("Enables recursive file-matching for cleaning entire directories of working files.\n"));
        help.endWrap();

        help.optionHead(_S("-i"), _S("follow implicit links"));
        help.beginWrap();
        help.wrap(_S("Enables implicit object traversal and cleaning. This is only useful if one or more paths ")
                  _S("are specified. For objects supporting implicit-gathering, this will query those ")
                  _S("objects for their current implicit links and ensure the linked-objects are cleaned ")
                  _S("as well.\n"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("clean");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_CLEAN
