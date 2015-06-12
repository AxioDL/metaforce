#ifndef CTOOL_EXTRACT
#define CTOOL_EXTRACT

#include "ToolBase.hpp"
#include <stdio.h>

class ToolExtract final : public ToolBase
{
public:
    ToolExtract(const ToolPassInfo& info)
    : ToolBase(info)
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-extract - Extract objects from supported package/image formats\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl extract <packagefile>[/<subnode>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command recursively extracts all or part of a dataspec-supported "
                     "package format.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<packagefile>[/<subnode>...]"), _S("input file"));
        help.beginWrap();
        help.wrap(_S("Specifies the package file or disc image to source data from. "
                     "An optional subnode specifies a named hierarchical-node specific "
                     "to the game architecture (levels/areas)."));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("extract");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_EXTRACT
