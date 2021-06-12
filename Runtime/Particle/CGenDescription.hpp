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

namespace metaforce {

class CGenDescription {
public:
  std::unique_ptr<CIntElement> x0_PSLT;
  std::unique_ptr<CIntElement> x4_PSWT;
  std::unique_ptr<CRealElement> x8_PSTS;
  std::unique_ptr<CVectorElement> xc_POFS;
  std::unique_ptr<CIntElement> x10_SEED;
  std::unique_ptr<CRealElement> x14_LENG;
  std::unique_ptr<CRealElement> x18_WIDT;
  std::unique_ptr<CIntElement> x1c_MAXP;
  std::unique_ptr<CRealElement> x20_GRTE;
  std::unique_ptr<CColorElement> x24_COLR;
  std::unique_ptr<CIntElement> x28_LTME;
  std::unique_ptr<CEmitterElement> x2c_EMTR;
  std::unique_ptr<CRealElement> x30_EADY; // Added in MP3
  bool x30_24_LINE : 1 = false;
  bool x30_25_FXLL : 1 = false;
  bool x30_26_AAPH : 1 = false;
  bool x30_27_ZBUF : 1 = false;
  bool x30_28_SORT : 1 = false;
  bool x30_29_LIT_ : 1 = false;
  bool x30_30_ORNT : 1 = false;
  bool x30_31_RSOP : 1 = false;
  bool x31_24_MBLR : 1 = false;
  bool x31_25_PMAB : 1 = false;
  bool x31_26_PMUS : 1 = false;
  bool x31_27_PMOO : 1 = true;
  bool x31_28_VMD1 : 1 = false;
  bool x31_29_VMD2 : 1 = false;
  bool x31_30_VMD3 : 1 = false;
  bool x31_31_VMD4 : 1 = false;
  bool x32_24_CIND : 1 = false;
  bool x32_25_OPTS : 1 = false;
  std::unique_ptr<CIntElement> x34_MBSP;
  std::unique_ptr<CRealElement> x38_SIZE;
  std::unique_ptr<CRealElement> x3c_ROTA;
  std::unique_ptr<CUVElement> x40_TEXR;
  std::unique_ptr<CUVElement> x44_TIND;
  SParticleModel x48_PMDL;
  std::unique_ptr<CVectorElement> x58_PMOP;
  std::unique_ptr<CVectorElement> x5c_PMRT;
  std::unique_ptr<CVectorElement> x60_PMSC;
  std::unique_ptr<CColorElement> x64_PMCL;
  std::unique_ptr<CModVectorElement> x68_VEL1;
  std::unique_ptr<CModVectorElement> x6c_VEL2;
  std::unique_ptr<CModVectorElement> x70_VEL3;
  std::unique_ptr<CModVectorElement> x74_VEL4;
  SChildGeneratorDesc x78_ICTS;
  std::unique_ptr<CIntElement> x88_NCSY;
  std::unique_ptr<CIntElement> x8c_CSSD;
  SChildGeneratorDesc x90_IDTS;
  std::unique_ptr<CIntElement> xa0_NDSY;
  SChildGeneratorDesc xa4_IITS;
  std::unique_ptr<CIntElement> xb4_PISY;
  std::unique_ptr<CIntElement> xb8_SISY;
  std::unique_ptr<CSpawnSystemKeyframeData> xbc_KSSM;
  SSwooshGeneratorDesc xc0_SSWH;
  std::unique_ptr<CIntElement> xd0_SSSD;
  std::unique_ptr<CVectorElement> xd4_SSPO;
  SElectricGeneratorDesc xd8_SELC;
  std::unique_ptr<CIntElement> xe4_SESD;
  std::unique_ptr<CVectorElement> xe8_SEPO;
  std::unique_ptr<CIntElement> xec_LTYP;
  std::unique_ptr<CColorElement> xf0_LCLR;
  std::unique_ptr<CRealElement> xf4_LINT;
  std::unique_ptr<CVectorElement> xf8_LOFF;
  std::unique_ptr<CVectorElement> xfc_LDIR;
  std::unique_ptr<CIntElement> x100_LFOT;
  std::unique_ptr<CRealElement> x104_LFOR;
  std::unique_ptr<CRealElement> x108_LSLA;
  std::unique_ptr<CRealElement> x10c_ADV1;
  std::unique_ptr<CRealElement> x110_ADV2;
  std::unique_ptr<CRealElement> x114_ADV3;
  std::unique_ptr<CRealElement> x118_ADV4;
  std::unique_ptr<CRealElement> x11c_ADV5;
  std::unique_ptr<CRealElement> x120_ADV6;
  std::unique_ptr<CRealElement> x124_ADV7;
  std::unique_ptr<CRealElement> x128_ADV8;
  std::unique_ptr<CRealElement> x164_ADV9;
  s32 x174_DFLG = 0; // Added in MP2
  s32 x178_ = 0; // In MP3, need to check MP2
  s32 x17c_ = 0; // In MP3, need to check MP2
  /* Custom additions */
  std::unique_ptr<CColorElement> m_bevelGradient; /* FourCC BGCL */

  CGenDescription() = default;
};

} // namespace metaforce
