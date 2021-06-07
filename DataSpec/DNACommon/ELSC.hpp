#pragma once

#include <vector>

#include "DataSpec/DNACommon/ParticleCommon.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

#include <athena/FileWriter.hpp>

namespace hecl {
class ProjectPath;
}

namespace DataSpec::DNAParticle {

template <class IDType>
struct _ELSM {
  static constexpr ParticleType Type = ParticleType::ELSM;

#define INT_ENTRY(name, identifier) IntElementFactory identifier;
#define REAL_ENTRY(name, identifier) RealElementFactory identifier;
#define COLOR_ENTRY(name, identifier) ColorElementFactory identifier;
#define EMITTER_ENTRY(name, identifier) EmitterElementFactory identifier;
#define RES_ENTRY(name, identifier) ChildResourceFactory<IDType> identifier;
#define BOOL_ENTRY(name, identifier) bool identifier = false;
#include "ELSC.def"

  template <typename _Func>
  void constexpr Enumerate(_Func f) {
#define ENTRY(name, identifier) f(FOURCC(name), identifier);
#include "ELSC.def"
  }

  template <typename _Func>
  bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
#define ENTRY(name, identifier)                                                                                        \
  case SBIG(name):                                                                                                     \
    f(identifier);                                                                                                     \
    return true;
#include "ELSC.def"
    default:
      return false;
    }
  }
};
template <class IDType>
using ELSM = PPImpl<_ELSM<IDType>>;

template <class IDType>
bool ExtractELSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteELSM(const ELSM<IDType>& elsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
