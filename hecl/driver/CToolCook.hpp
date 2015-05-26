#ifndef CTOOL_COOK
#define CTOOL_COOK

#include "CToolBase.hpp"
#include <stdio.h>

class CToolCook final : public CToolBase
{
public:
    CToolCook(const SToolPassInfo& info)
    : CToolBase(info)
    {
    }

    ~CToolCook()
    {
    }

    static void Help(CHelpOutput& help)
    {
        help.secHead("NAME");
        help.beginWrap();
        help.wrap("hecl-cook - Cook objects within the project database\n");
        help.endWrap();

        help.secHead("SYNOPSIS");
        help.beginWrap();
        help.wrap("hecl cook [-r] [<pathspec>...]\n");
        help.endWrap();

        help.secHead("DESCRIPTION");
        help.beginWrap();
        help.wrap("This command initiates a cooking pass on the project database. Cooking "
                  "is analogous to compiling in software development. The resulting object buffers "
                  "are cached within the project database. HECL performs the following "
                  "tasks for each object during the cook process:\n\n");
        help.wrapBold("- Object Gather: ");
        help.wrap("Files added with ");
        help.wrapBold("hecl add");
        help.wrap(" are queried for their dependent files (e.g. ");
        help.wrapBold(".blend");
        help.wrap(" files return any linked ");
        help.wrapBold(".png");
        help.wrap(" images). If the dependent files are unable to be found, the cook process aborts.\n\n");
        help.wrapBold("- Modtime Comparison: ");
        help.wrap("Files that have previously finished a cook pass are inspected for their time of "
                  "last modification. If the file hasn't changed since its previous cook-pass, the "
                  "process is skipped. If the file has been moved or deleted, the object is automatically "
                  "removed from the project database.\n\n");
        help.wrapBold("- Cook: ");
        help.wrap("A type-specific procedure compiles the file's contents into an efficient format "
                  "for use by the runtime. A data-buffer is provided to HECL.\n\n");
        help.wrapBold("- Hash and Compress: ");
        help.wrap("The data-buffer is hashed and compressed before being cached in the object database.\n\n");
        help.endWrap();

        help.secHead("OPTIONS");
        help.optionHead("<pathspec>...", "input file(s)");
        help.beginWrap();
        help.wrap("Specifies working file(s) containing production data to be cooked by HECL. "
                  "Glob-strings may be specified (e.g. ");
        help.wrapBold("*.blend");
        help.wrap(") to automatically cook all matching current-directory files in the project database. "
                  "If no path specified, all files in the project database are cooked.\n");
        help.endWrap();

        help.optionHead("-r", "recursion");
        help.beginWrap();
        help.wrap("Enables recursive file-matching for cooking entire directories of working files.\n");
        help.endWrap();
    }

    std::string toolName() const {return "cook";}

    int run()
    {
        return 0;
    }
};

#endif // CTOOL_COOK
