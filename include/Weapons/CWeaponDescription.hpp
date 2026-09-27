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

  const TCollisionResponseDesc& GetCollisionResponse() const { return mCOLR; }
  const TParticleModel& GetOHEF() const { return mOHEF; }
  static float GetTickPeriod();
  CWeaponDescription();
  ~CWeaponDescription();
  CVectorElement* mIORN;
  CVectorElement* mIVEC;
  CVectorElement* mPSOV;
  CModVectorElement* mPSVM;
  bool mVMD2;
  CIntElement* mPSLT;
  CVectorElement* mPSCL;
  CColorElement* mPCOL;
  CVectorElement* mPOFS;
  CVectorElement* mOFST;
#if VERSION >= VERSION_GM8P_00
  bool mAPSO : 1;
  bool mF60H : 1;
  bool mSVBD : 1;
  bool mNDTT : 1;
  bool mHOMG : 1;
  bool mAP11 : 1;
  bool mSPS1 : 1;
  bool mAP21 : 1;
  bool mSPS2 : 1;
  bool mAS11 : 1;
  bool mAS12 : 1;
  bool mAS13 : 1;
  bool mEWTR : 1;
  bool mLWTR : 1;
  bool mSWTR : 1;
  bool mFC60 : 1;
#else
  bool mAPSO;
  bool mHOMG;
  bool mAP11;
  bool mAP21;
  bool mAS11;
  bool mAS12;
  bool mAS13;
#endif
  CRealElement* mTRAT;
  TChildGeneratorDesc mAPSM;
  TChildGeneratorDesc mAPS2;
  TSwooshGeneratorDesc mASW1;
  TSwooshGeneratorDesc mASW2;
  TSwooshGeneratorDesc mASW3;
  TParticleModel mOHEF;
  TCollisionResponseDesc mCOLR;
#if VERSION < VERSION_GM8P_00
  bool mEWTR;
  bool mLWTR;
  bool mSWTR;
#endif
  int mPJFX;
  CRealElement* mRNGE;
  CRealElement* mFOFF;
};
CHECK_SIZEOF(CWeaponDescription,
             (VERSION >= VERSION_GM8P_00 ? 0xac : 0xb4))

#endif // _CWEAPONDESCRIPTION
