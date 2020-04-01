#pragma once

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
struct _SWSH {
  static constexpr ParticleType Type = ParticleType::SWSH;

#define INT_ENTRY(name, identifier) IntElementFactory identifier;
#define REAL_ENTRY(name, identifier) RealElementFactory identifier;
#define VECTOR_ENTRY(name, identifier) VectorElementFactory identifier;
#define MOD_VECTOR_ENTRY(name, identifier) ModVectorElementFactory identifier;
#define COLOR_ENTRY(name, identifier) ColorElementFactory identifier;
#define UV_ENTRY(name, identifier) UVElementFactory<IDType> identifier;
#define BOOL_ENTRY(name, identifier, def) bool identifier = def;
#include "SWHC.def"

  template<typename _Func>
  void constexpr Enumerate(_Func f) {
#define ENTRY(name, identifier) f(FOURCC(name), identifier);
#define BOOL_ENTRY(name, identifier, def) f(FOURCC(name), identifier, def);
#include "SWHC.def"
  }

  template<typename _Func>
  bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
#define ENTRY(name, identifier) case SBIG(name): f(identifier); return true;
#include "SWHC.def"
    default: return false;
    }
  }
};
template <class IDType>
using SWSH = PPImpl<_SWSH<IDType>>;

template <class IDType>
bool ExtractSWSH(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteSWSH(const SWSH<IDType>& gpsm, const hecl::ProjectPath& outPath);
} // namespace DataSpec::DNAParticle
