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
struct ELSM : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  AT_SUBDECL_DNA
  IntElementFactory x0_LIFE;
  IntElementFactory x4_SLIF;
  RealElementFactory x8_GRAT;
  IntElementFactory xc_SCNT;
  IntElementFactory x10_SSEG;
  ColorElementFactory x14_COLR;
  EmitterElementFactory x18_IEMT;
  EmitterElementFactory x1c_FEMT;
  RealElementFactory x20_AMPL;
  RealElementFactory x24_AMPD;
  RealElementFactory x28_LWD1;
  RealElementFactory x2c_LWD2;
  RealElementFactory x30_LWD3;
  ColorElementFactory x34_LCL1;
  ColorElementFactory x38_LCL2;
  ColorElementFactory x3c_LCL3;
  ChildResourceFactory<IDType> x40_SSWH;
  ChildResourceFactory<IDType> x50_GPSM;
  ChildResourceFactory<IDType> x60_EPSM;
  BoolHelper x70_ZERY;

  void gatherDependencies(std::vector<hecl::ProjectPath>&) const;
};

template <class IDType>
bool ExtractELSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteELSM(const ELSM<IDType>& elsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
