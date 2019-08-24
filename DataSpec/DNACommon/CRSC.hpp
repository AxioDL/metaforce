#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/ParticleCommon.hpp"

#include <athena/DNA.hpp>

namespace DataSpec {
class PAKEntryReadStream;
}

namespace hecl {
class ProjectPath;
}

namespace DataSpec::DNAParticle {
template <class IDType>
struct CRSM : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  AT_SUBDECL_DNA
  std::unordered_map<FourCC, ChildResourceFactory<IDType>> x0_generators;
  std::unordered_map<FourCC, uint32_t> x10_sfx;
  std::unordered_map<FourCC, ChildResourceFactory<IDType>> x20_decals;
  float x30_RNGE;
  float x34_FOFF;

  CRSM();

  void gatherDependencies(std::vector<hecl::ProjectPath>&) const;
};
template <class IDType>
bool ExtractCRSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteCRSM(const CRSM<IDType>& crsm, const hecl::ProjectPath& outPath);
} // namespace DataSpec::DNAParticle
