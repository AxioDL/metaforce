#pragma once

#include <cstdint>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/ParticleCommon.hpp"

namespace DataSpec {
class PAKEntryReadStream;
}

namespace hecl {
class ProjectPath;
}

namespace DataSpec::DNAParticle {

template <class IDType>
struct _GPSM {
  static constexpr ParticleType Type = ParticleType::GPSM;

#define INT_ENTRY(name, identifier) IntElementFactory identifier;
#define REAL_ENTRY(name, identifier) RealElementFactory identifier;
#define VECTOR_ENTRY(name, identifier) VectorElementFactory identifier;
#define MOD_VECTOR_ENTRY(name, identifier) ModVectorElementFactory identifier;
#define COLOR_ENTRY(name, identifier) ColorElementFactory identifier;
#define EMITTER_ENTRY(name, identifier) EmitterElementFactory identifier;
#define UV_ENTRY(name, identifier) UVElementFactory<IDType> identifier;
#define RES_ENTRY(name, identifier) ChildResourceFactory<IDType> identifier;
#define KSSM_ENTRY(name, identifier) SpawnSystemKeyframeData<IDType> identifier;
#define BOOL_ENTRY(name, identifier, def) bool identifier = def;
#include "PART.def"

  template<typename _Func>
  void constexpr Enumerate(_Func f) {
#define ENTRY(name, identifier) f(FOURCC(name), identifier);
#define BOOL_ENTRY(name, identifier, def) f(FOURCC(name), identifier, def);
#include "PART.def"
  }

  template<typename _Func>
  bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
#define ENTRY(name, identifier) case SBIG(name): f(identifier); return true;
#include "PART.def"
    default: return false;
    }
  }
};
template <class IDType>
using GPSM = PPImpl<_GPSM<IDType>>;

template <class IDType>
bool ExtractGPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteGPSM(const GPSM<IDType>& gpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
