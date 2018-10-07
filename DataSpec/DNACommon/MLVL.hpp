#pragma once

#include "DNACommon.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec::DNAMLVL
{

template <class PAKRouter, typename MLVL>
bool ReadMLVLToBlender(hecl::blender::Connection& conn,
                       const MLVL& mlvl,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       bool force,
                       std::function<void(const hecl::SystemChar*)> fileChanged);

}

