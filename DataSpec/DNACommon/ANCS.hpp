#pragma once

#include <unordered_set>
#include "DNACommon.hpp"
#include "CMDL.hpp"
#include "RigInverter.hpp"

namespace DataSpec::DNAANCS
{

using Actor = hecl::blender::Actor;
using Armature = hecl::blender::Armature;
using Action = hecl::blender::Action;

template <typename IDTYPE>
struct CharacterResInfo
{
    std::string name;
    IDTYPE cmdl;
    IDTYPE cskr;
    IDTYPE cinf;
    std::vector<std::pair<hecl::FourCC, std::pair<IDTYPE, IDTYPE>>> overlays;
};

template <typename IDTYPE>
struct AnimationResInfo
{
    std::string name;
    IDTYPE animId;
    IDTYPE evntId;
    bool additive;
};

template <class PAKRouter, class ANCSDNA, class MaterialSet, class SurfaceHeader, atUint32 CMDLVersion>
bool ReadANCSToBlender(hecl::blender::Connection& conn,
                       const ANCSDNA& ancs,
                       const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       const SpecBase& dataspec,
                       std::function<void(const hecl::SystemChar*)> fileChanged,
                       bool force=false);

}

