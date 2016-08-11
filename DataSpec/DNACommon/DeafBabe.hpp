#ifndef _DNACOMMON_DEAFBABE_HPP_
#define _DNACOMMON_DEAFBABE_HPP_

#include "DNACommon.hpp"
#include "BlenderConnection.hpp"

namespace DataSpec
{

enum class BspNodeType : atUint32
{
    Invalid,
    Branch,
    Leaf
};

template<class DEAFBABE>
void DeafBabeSendToBlender(hecl::BlenderConnection::PyOutStream& os, const DEAFBABE& db);

template<class DEAFBABE>
void DeafBabeBuildFromBlender(DEAFBABE& db, const hecl::BlenderConnection::DataStream::ColMesh& colMesh);

}

#endif // _DNACOMMON_DEAFBABE_HPP_
