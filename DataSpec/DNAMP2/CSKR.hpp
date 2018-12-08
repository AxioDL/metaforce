#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "CINF.hpp"
#include "../DNAMP1/CSKR.hpp"

namespace DataSpec::DNAMP2 {

struct CSKR : DNAMP1::CSKR {
  Delete expl;

  const atInt16* getMatrixBank(size_t) const { return nullptr; }

  void weightVertex(hecl::blender::PyOutStream& os, const CINF& cinf, atUint32 idx) const;
};

} // namespace DataSpec::DNAMP2
