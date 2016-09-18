#ifndef _DNACOMMON_ATBL_HPP_
#define _DNACOMMON_ATBL_HPP_

#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec
{
namespace DNAAudio
{

class ATBL
{
public:
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}
}

#endif // _DNACOMMON_ATBL_HPP_
