#ifndef _DNACOMMON_DEAFBABE_HPP_
#define _DNACOMMON_DEAFBABE_HPP_

#include "DNACommon.hpp"

namespace DataSpec
{

enum class BspNodeType : atUint32
{
    Invalid,
    Branch,
    Leaf
};

template<class DEAFBABE>
void DeafBabeSendToBlender(hecl::blender::PyOutStream& os, const DEAFBABE& db, bool isDcln = false, atInt32 idx = -1);

template<class DEAFBABE>
void DeafBabeBuildFromBlender(DEAFBABE& db, const hecl::blender::ColMesh& colMesh);

}

#endif // _DNACOMMON_DEAFBABE_HPP_
