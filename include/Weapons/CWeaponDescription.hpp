#ifndef _CWEAPONDESCRIPTION
#define _CWEAPONDESCRIPTION

#include "Kyoto/Particles/CModVectorElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CModel;
class CElectricDescription;
class CSwooshDescription;
class CGenDescription;
class CCollisionResponseData;

class CWeaponDescription {
public:
  typedef rstl::optional_object< TLockedToken< CModel > > TParticleModel;
  typedef rstl::optional_object< TLockedToken< CGenDescription > > TChildGeneratorDesc;
  typedef rstl::optional_object< TLockedToken< CSwooshDescription > > TSwooshGeneratorDesc;
  typedef rstl::optional_object< TLockedToken< CCollisionResponseData > > TCollisionResponseDesc;

  const TCollisionResponseDesc& GetCollisionResponse() const { return x94_COLR; }
  const TParticleModel& GetOHEF() const { return x84_OHEF; }
  static float GetTickPeriod();
  CWeaponDescription();
  ~CWeaponDescription();
  CVectorElement* x0_IORN;
  CVectorElement* x4_IVEC;
  CVectorElement* x8_PSOV;
  CModVectorElement* xc_PSVM;
  bool x10_VMD2;
  CIntElement* x14_PSLT;
  CVectorElement* x18_PSCL;
  CColorElement* x1c_PCOL;
  CVectorElement* x20_POFS;
  CVectorElement* x24_OFST;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  bool x28_APSO : 1;
  bool x28_25_F60H : 1;
  bool x28_26_SVBD : 1;
  bool x28_27_NDTT : 1;
  bool x29_HOMG : 1;
  bool x2a_AP11 : 1;
  bool x28_30_SPS1 : 1;
  bool x2b_AP21 : 1;
  bool x29_24_SPS2 : 1;
  bool x2c_AS11 : 1;
  bool x2d_AS12 : 1;
  bool x2e_AS13 : 1;
  bool xa4_EWTR : 1;
  bool xa5_LWTR : 1;
  bool xa6_SWTR : 1;
  bool x29_31_FC60 : 1;
#else
  bool x28_APSO;
  bool x29_HOMG;
  bool x2a_AP11;
  bool x2b_AP21;
  bool x2c_AS11;
  bool x2d_AS12;
  bool x2e_AS13;
#endif
  CRealElement* x30_TRAT;
  TChildGeneratorDesc x34_APSM;
  TChildGeneratorDesc x44_APS2;
  TSwooshGeneratorDesc x54_ASW1;
  TSwooshGeneratorDesc x64_ASW2;
  TSwooshGeneratorDesc x74_ASW3;
  TParticleModel x84_OHEF;
  TCollisionResponseDesc x94_COLR;
#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02
  bool xa4_EWTR;
  bool xa5_LWTR;
  bool xa6_SWTR;
#endif
  int xa8_PJFX;
  CRealElement* xac_RNGE;
  CRealElement* xb0_FOFF;
};
CHECK_SIZEOF(CWeaponDescription,
             (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0xac : 0xb4))

#endif // _CWEAPONDESCRIPTION
