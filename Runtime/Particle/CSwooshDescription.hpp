#pragma once

#include "CParticleDataFactory.hpp"
#include "CRealElement.hpp"
#include "CIntElement.hpp"
#include "CVectorElement.hpp"
#include "CModVectorElement.hpp"
#include "CColorElement.hpp"
#include "CUVElement.hpp"

namespace urde {
class CSwooshDescription {
public:
  std::unique_ptr<CIntElement> x0_PSLT;
  std::unique_ptr<CRealElement> x4_TIME;
  std::unique_ptr<CRealElement> x8_LRAD;
  std::unique_ptr<CRealElement> xc_RRAD;
  std::unique_ptr<CIntElement> x10_LENG;
  std::unique_ptr<CColorElement> x14_COLR;
  std::unique_ptr<CIntElement> x18_SIDE;
  std::unique_ptr<CRealElement> x1c_IROT;
  std::unique_ptr<CRealElement> x20_ROTM;
  std::unique_ptr<CVectorElement> x24_POFS;
  std::unique_ptr<CVectorElement> x28_IVEL;
  std::unique_ptr<CVectorElement> x2c_NPOS;
  std::unique_ptr<CModVectorElement> x30_VELM;
  std::unique_ptr<CModVectorElement> x34_VLM2;
  std::unique_ptr<CIntElement> x38_SPLN;
  std::unique_ptr<CUVElement> x3c_TEXR;
  std::unique_ptr<CIntElement> x40_TSPN;
  union {
    struct {
      bool x44_24_LLRD : 1;
      bool x44_25_CROS : 1;
      bool x44_26_VLS1 : 1;
      bool x44_27_VLS2 : 1;
      bool x44_28_SROT : 1;
      bool x44_29_WIRE : 1;
      bool x44_30_TEXW : 1;
      bool x44_31_AALP : 1;
      bool x45_24_ZBUF : 1;
      bool x45_25_ORNT : 1;
      bool x45_26_CRND : 1;
    };
    u32 dummy = 0;
  };

  CSwooshDescription() { x44_25_CROS = true; }
};
} // namespace urde
