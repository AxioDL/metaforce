#ifndef __DNACOMMON_MLVL_HPP__
#define __DNACOMMON_MLVL_HPP__

#include "DNACommon.hpp"
#include "BlenderConnection.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec
{
namespace DNAMLVL
{

template <class PAKRouter, typename MLVL>
bool ReadMLVLToBlender(hecl::BlenderConnection& conn,
                       const MLVL& mlvl,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force,
                       std::function<void(const hecl::SystemChar*)> fileChanged);

}
}

#endif // __DNACOMMON_MLVL_HPP__
