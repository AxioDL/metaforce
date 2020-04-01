#pragma once

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
struct _WPSM {
  static constexpr ParticleType Type = ParticleType::WPSM;

#define INT_ENTRY(name, identifier) IntElementFactory identifier;
#define U32_ENTRY(name, identifier) uint32_t identifier = ~0;
#define REAL_ENTRY(name, identifier) RealElementFactory identifier;
#define VECTOR_ENTRY(name, identifier) VectorElementFactory identifier;
#define MOD_VECTOR_ENTRY(name, identifier) ModVectorElementFactory identifier;
#define COLOR_ENTRY(name, identifier) ColorElementFactory identifier;
#define UV_ENTRY(name, identifier) UVElementFactory<IDType> identifier;
#define RES_ENTRY(name, identifier) ChildResourceFactory<IDType> identifier;
#define BOOL_ENTRY(name, identifier, def) bool identifier = def;
#include "WPSC.def"

  template<typename _Func>
  void constexpr Enumerate(_Func f) {
#define ENTRY(name, identifier) f(FOURCC(name), identifier);
#define BOOL_ENTRY(name, identifier, def) f(FOURCC(name), identifier, def);
#include "WPSC.def"
  }

  template<typename _Func>
  bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
#define ENTRY(name, identifier) case SBIG(name): f(identifier); return true;
#include "WPSC.def"
    default: return false;
    }
  }
};
template <class IDType>
using WPSM = PPImpl<_WPSM<IDType>>;

template <class IDType>
bool ExtractWPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteWPSM(const WPSM<IDType>& wpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
