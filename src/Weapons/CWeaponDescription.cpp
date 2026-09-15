#include "Weapons/CWeaponDescription.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

CWeaponDescription::CWeaponDescription()
: x0_IORN(nullptr)
, x4_IVEC(nullptr)
, x8_PSOV(nullptr)
, xc_PSVM(nullptr)
, x10_VMD2(false)
, x14_PSLT(nullptr)
, x18_PSCL(nullptr)
, x1c_PCOL(nullptr)
, x20_POFS(nullptr)
, x24_OFST(nullptr)
, x28_APSO(false)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, x28_25_F60H(false)
, x28_26_SVBD(false)
, x28_27_NDTT(false)
#endif
, x2a_AP11(false)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, x28_30_SPS1(false)
#endif
, x2b_AP21(false)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, x29_24_SPS2(false)
#endif
, x2c_AS11(false)
, x2d_AS12(false)
, x2e_AS13(false)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
, xa4_EWTR(true)
, xa5_LWTR(true)
, xa6_SWTR(true)
, x29_31_FC60(false)
, x30_TRAT(nullptr)
#else
, x30_TRAT(nullptr)
, xa4_EWTR(true)
, xa5_LWTR(true)
, xa6_SWTR(true)
#endif
, xa8_PJFX(-1)
, xac_RNGE(nullptr)
, xb0_FOFF(nullptr) {
}

CWeaponDescription::~CWeaponDescription() {
  delete x4_IVEC;
  delete x0_IORN;
  delete xc_PSVM;
  delete x8_PSOV;
  delete x14_PSLT;
  delete x18_PSCL;
  delete x1c_PCOL;
  delete x20_POFS;
  delete x24_OFST;
  delete x30_TRAT;
  delete xac_RNGE;
  delete xb0_FOFF;
}
