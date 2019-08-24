#pragma once

#include <functional>

#include "DataSpec/DNACommon/DNACommon.hpp"

#include <hecl/SystemChar.hpp>

namespace hecl {
class ProjectPath;
}

namespace hecl::blender {
class Connection;
}

namespace DataSpec::DNAMLVL {

template <class PAKRouter, typename MLVL>
bool ReadMLVLToBlender(hecl::blender::Connection& conn, const MLVL& mlvl, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, bool force,
                       std::function<void(const hecl::SystemChar*)> fileChanged);

}
