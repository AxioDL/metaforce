#ifndef __DNACOMMON_ITWEAK_HPP__
#define __DNACOMMON_ITWEAK_HPP__

#include "../DNACommon.hpp"

namespace hecl
{
class CVarManager;
}
namespace DataSpec
{
struct ITweak : BigDNA
{

    virtual void initCVars(hecl::CVarManager*) {}
};
}

#endif // __DNACOMMON_ITWEAK_HPP__
