#ifndef __DNACOMMON_MLVL_HPP__
#define __DNACOMMON_MLVL_HPP__

#include "DNACommon.hpp"
#include "BlenderConnection.hpp"
#include "CVector3f.hpp"

namespace DataSpec
{
namespace DNAMLVL
{

template <class PAKRouter, typename MLVL>
bool ReadMLVLToBlender(HECL::BlenderConnection& conn,
                       const MLVL& mlvl,
                       const HECL::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force,
                       std::function<void(const HECL::SystemChar*)> fileChanged);

}
}

#endif // __DNACOMMON_MLVL_HPP__
