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
struct _CRSM {
  static constexpr ParticleType Type = ParticleType::CRSM;
#define RES_ENTRY(name, identifier) ChildResourceFactory<IDType> identifier;
#define U32_ENTRY(name, identifier) uint32_t identifier = ~0;
#define FLOAT_ENTRY(name, identifier) float identifier = 0.f;
#include "CRSC.def"

  template <typename _Func>
  void constexpr Enumerate(_Func f) {
#define ENTRY(name, identifier) f(FOURCC(name), identifier);
#include "CRSC.def"
  }

  template <typename _Func>
  bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
#define ENTRY(name, identifier)                                                                                        \
  case SBIG(name):                                                                                                     \
    f(identifier);                                                                                                     \
    return true;
#include "CRSC.def"
    default:
      return false;
    }
  }
};
template <class IDType>
using CRSM = PPImpl<_CRSM<IDType>>;

template <class IDType>
bool ExtractCRSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteCRSM(const CRSM<IDType>& crsm, const hecl::ProjectPath& outPath);
} // namespace DataSpec::DNAParticle
