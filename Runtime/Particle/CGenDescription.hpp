#pragma once

#include <memory>

#include "Runtime/Particle/CColorElement.hpp"
#include "Runtime/Particle/CEmitterElement.hpp"
#include "Runtime/Particle/CIntElement.hpp"
#include "Runtime/Particle/CModVectorElement.hpp"
#include "Runtime/Particle/CParticleDataFactory.hpp"
#include "Runtime/Particle/CRealElement.hpp"
#include "Runtime/Particle/CSpawnSystemKeyframeData.hpp"
#include "Runtime/Particle/CUVElement.hpp"
#include "Runtime/Particle/CVectorElement.hpp"

/* Documentation at: https://wiki.axiodl.com/w/PART_(File_Format) */

namespace urde {

class CGenDescription {
public:
  /* Naming convention: <demo-offset>_<retail-offset>_<name> */

  /* Removed from demo */
  // std::unique_ptr<CVectorElement> x0_PSIV;
  // std::unique_ptr<CModVectorElement> x4_PSVM;
  // std::unique_ptr<CVectorElement> x8_PSOV;
  std::unique_ptr<CIntElement> xc_x0_PSLT;
  std::unique_ptr<CIntElement> x10_x4_PSWT;
  std::unique_ptr<CRealElement> x14_x8_PSTS;
  std::unique_ptr<CVectorElement> x18_xc_POFS;
  std::unique_ptr<CIntElement> x1c_x10_SEED;
  std::unique_ptr<CRealElement> x20_x14_LENG;
  std::unique_ptr<CRealElement> x24_x18_WIDT;
  std::unique_ptr<CIntElement> x28_x1c_MAXP;
  std::unique_ptr<CRealElement> x2c_x20_GRTE;
  std::unique_ptr<CColorElement> x30_x24_COLR;
  std::unique_ptr<CIntElement> x34_x28_LTME;
  /* Removed from demo (replaced by EMTR) */
  // std::unique_ptr<CVectorElement> x38_ILOC;
  // std::unique_ptr<CVectorElement> x3c_IVEC;
  std::unique_ptr<CEmitterElement> x40_x2c_EMTR;
  bool x44_28_x30_28_SORT : 1 = false;
  bool x44_30_x31_24_MBLR : 1 = false;
  bool x44_24_x30_24_LINE : 1 = false;
  bool x44_29_x30_29_LIT_ : 1 = false;
  bool x44_26_x30_26_AAPH : 1 = false;
  bool x44_27_x30_27_ZBUF : 1 = false;
  bool x44_25_x30_25_FXLL : 1 = false;
  bool x44_31_x31_25_PMAB : 1 = false;
  bool x45_29_x31_31_VMD4 : 1 = false;
  bool x45_28_x31_30_VMD3 : 1 = false;
  bool x45_27_x31_29_VMD2 : 1 = false;
  bool x45_26_x31_28_VMD1 : 1 = false;
  bool x45_31_x32_25_OPTS : 1 = false;
  bool x45_24_x31_26_PMUS : 1 = false;
  bool x45_25_x31_27_PMOO : 1 = true;
  bool x45_30_x32_24_CIND : 1 = false;
  /* 0-00 additions */
  bool x30_30_ORNT : 1 = false;
  bool x30_31_RSOP : 1 = false;
  std::unique_ptr<CIntElement> x48_x34_MBSP;
  std::unique_ptr<CRealElement> x4c_x38_SIZE;
  std::unique_ptr<CRealElement> x50_x3c_ROTA;
  std::unique_ptr<CUVElement> x54_x40_TEXR;
  std::unique_ptr<CUVElement> x58_x44_TIND;
  SParticleModel x5c_x48_PMDL;
  std::unique_ptr<CVectorElement> x6c_x58_PMOP;
  std::unique_ptr<CVectorElement> x70_x5c_PMRT;
  std::unique_ptr<CVectorElement> x74_x60_PMSC;
  std::unique_ptr<CColorElement> x78_x64_PMCL;
  std::unique_ptr<CModVectorElement> x7c_x68_VEL1;
  std::unique_ptr<CModVectorElement> x80_x6c_VEL2;
  std::unique_ptr<CModVectorElement> x84_x70_VEL3;
  std::unique_ptr<CModVectorElement> x88_x74_VEL4;
  SChildGeneratorDesc x8c_x78_ICTS;
  std::unique_ptr<CIntElement> x9c_x88_NCSY;
  std::unique_ptr<CIntElement> xa0_x8c_CSSD;
  SChildGeneratorDesc xa4_x90_IDTS;
  std::unique_ptr<CIntElement> xb4_xa0_NDSY;
  SChildGeneratorDesc xb8_xa4_IITS;
  std::unique_ptr<CIntElement> xc8_xb4_PISY;
  std::unique_ptr<CIntElement> xcc_xb8_SISY;
  std::unique_ptr<CSpawnSystemKeyframeData> xd0_xbc_KSSM;
  SSwooshGeneratorDesc xd4_xc0_SSWH;
  std::unique_ptr<CIntElement> xe4_xd0_SSSD;
  std::unique_ptr<CVectorElement> xe8_xd4_SSPO;
  SElectricGeneratorDesc xec_xd8_SELC;
  std::unique_ptr<CIntElement> xf8_xe4_SESD;
  std::unique_ptr<CVectorElement> xfc_xe8_SEPO;
  std::unique_ptr<CIntElement> x100_xec_LTYP;
  std::unique_ptr<CColorElement> x104_xf0_LCLR;
  std::unique_ptr<CRealElement> x108_xf4_LINT;
  std::unique_ptr<CVectorElement> x10c_xf8_LOFF;
  std::unique_ptr<CVectorElement> x110_xfc_LDIR;
  std::unique_ptr<CIntElement> x114_x100_LFOT;
  std::unique_ptr<CRealElement> x118_x104_LFOR;
  std::unique_ptr<CRealElement> x11c_x108_LSLA;
  std::unique_ptr<CRealElement> x10c_ADV1;
  std::unique_ptr<CRealElement> x110_ADV2;
  std::unique_ptr<CRealElement> x114_ADV3;
  std::unique_ptr<CRealElement> x118_ADV4;
  std::unique_ptr<CRealElement> x11c_ADV5;
  std::unique_ptr<CRealElement> x120_ADV6;
  std::unique_ptr<CRealElement> x124_ADV7;
  std::unique_ptr<CRealElement> x128_ADV8;

  /* Custom additions */
  std::unique_ptr<CColorElement> m_bevelGradient; /* FourCC BGCL */

  CGenDescription() = default;
};

} // namespace urde
