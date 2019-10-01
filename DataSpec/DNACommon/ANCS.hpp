#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "athena/Types.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/SystemChar.hpp"

namespace DataSpec {
struct SpecBase;
}

namespace hecl {
class ProjectPath;
}

namespace DataSpec::DNAANCS {

using Actor = hecl::blender::Actor;
using Armature = Actor::ActorArmature;
using Action = hecl::blender::Action;

template <typename IDTYPE>
struct CharacterResInfo {
  std::string name;
  IDTYPE cmdl;
  IDTYPE cskr;
  IDTYPE cinf;
  std::vector<std::pair<std::string, std::pair<IDTYPE, IDTYPE>>> overlays;
};

template <typename IDTYPE>
struct AnimationResInfo {
  std::string name;
  IDTYPE animId;
  IDTYPE evntId;
  bool additive;
};

template <class PAKRouter, class ANCSDNA, class MaterialSet, class SurfaceHeader, atUint32 CMDLVersion>
bool ReadANCSToBlender(hecl::blender::Connection& conn, const ANCSDNA& ancs, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, const SpecBase& dataspec,
                       std::function<void(const hecl::SystemChar*)> fileChanged, bool force = false);

} // namespace DataSpec::DNAANCS
