#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/ParticleCommon.hpp"

#include <athena/FileWriter.hpp>

namespace DataSpec {
class PAKEntryReadStream;
}

namespace hecl {
class ProjectPath;
}

namespace DataSpec::DNAParticle {

template <class IDType>
struct _DPSM {
  static constexpr ParticleType Type = ParticleType::DPSM;

  struct SQuadDescr {
    IntElementFactory x0_LFT;
    RealElementFactory x4_SZE;
    RealElementFactory x8_ROT;
    VectorElementFactory xc_OFF;
    ColorElementFactory x10_CLR;
    UVElementFactory<IDType> x14_TEX;
    bool x18_ADD = false;
  };

  SQuadDescr x0_quad;
  SQuadDescr x1c_quad;
  ChildResourceFactory<IDType> x38_DMDL;
  IntElementFactory x48_DLFT;
  VectorElementFactory x4c_DMOP;
  VectorElementFactory x50_DMRT;
  VectorElementFactory x54_DMSC;
  ColorElementFactory x58_DMCL;

  bool x5c_24_DMAB = false;
  bool x5c_25_DMOO = false;

  template<typename _Func>
  void constexpr Enumerate(_Func f) {
#define ENTRY(name, identifier) f(FOURCC(name), identifier);
#include "DPSC.def"
  }

  template<typename _Func>
  bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
#define ENTRY(name, identifier) case SBIG(name): f(identifier); return true;
#include "DPSC.def"
    default: return false;
    }
  }
};
template <class IDType>
using DPSM = PPImpl<_DPSM<IDType>>;

template <class IDType>
bool ExtractDPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteDPSM(const DPSM<IDType>& dpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
