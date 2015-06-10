#ifndef CTOOL_COOK
#define CTOOL_COOK

#include "ToolBase.hpp"
#include <stdio.h>

class ToolCook final : public ToolBase
{
public:
    ToolCook(const ToolPassInfo& info)
    : ToolBase(info)
    {
    }

    ~ToolCook()
    {
    }

    static void Help(HelpOutput& help)
    {
        help.secHead(_S("NAME"));
        help.beginWrap();
        help.wrap(_S("hecl-cook - Cook objects within the project database\n"));
        help.endWrap();

        help.secHead(_S("SYNOPSIS"));
        help.beginWrap();
        help.wrap(_S("hecl cook [-r] [<pathspec>...]\n"));
        help.endWrap();

        help.secHead(_S("DESCRIPTION"));
        help.beginWrap();
        help.wrap(_S("This command initiates a cooking pass on the project database. Cooking "
                  "is analogous to compiling in software development. The resulting object buffers "
                  "are cached within the project database. HECL performs the following "
                  "tasks for each object during the cook process:\n\n"));
        help.wrapBold(_S("- Object Gather: "));
        help.wrap(_S("Files added with "));
        help.wrapBold(_S("hecl add"));
        help.wrap(_S(" are queried for their dependent files (e.g. "));
        help.wrapBold(_S(".blend"));
        help.wrap(_S(" files return any linked "));
        help.wrapBold(_S(".png"));
        help.wrap(_S(" images). If the dependent files are unable to be found, the cook process aborts.\n\n"));
        help.wrapBold(_S("- Modtime Comparison: "));
        help.wrap(_S("Files that have previously finished a cook pass are inspected for their time of "
                  "last modification. If the file hasn't changed since its previous cook-pass, the "
                  "process is skipped. If the file has been moved or deleted, the object is automatically "
                  "removed from the project database.\n\n"));
        help.wrapBold(_S("- Cook: "));
        help.wrap(_S("A type-specific procedure compiles the file's contents into an efficient format "
                  "for use by the runtime. A data-buffer is provided to HECL.\n\n"));
        help.wrapBold(_S("- Hash and Compress: "));
        help.wrap(_S("The data-buffer is hashed and compressed before being cached in the object database.\n\n"));
        help.endWrap();

        help.secHead(_S("OPTIONS"));
        help.optionHead(_S("<pathspec>..."), _S("input file(s)"));
        help.beginWrap();
        help.wrap(_S("Specifies working file(s) containing production data to be cooked by HECL. "
                  "Glob-strings may be specified (e.g. "));
        help.wrapBold(_S("*.blend"));
        help.wrap(_S(") to automatically cook all matching current-directory files in the project database. "
                  "If no path specified, all files in the project database are cooked.\n"));
        help.endWrap();

        help.optionHead(_S("-r"), _S("recursion"));
        help.beginWrap();
        help.wrap(_S("Enables recursive file-matching for cooking entire directories of working files.\n"));
        help.endWrap();
    }

    HECL::SystemString toolName() const {return _S("cook");}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_COOK
