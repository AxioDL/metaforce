#include "Weapons/CWeaponDescription.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

CWeaponDescription::CWeaponDescription()
: mIORN(nullptr)
, mIVEC(nullptr)
, mPSOV(nullptr)
, mPSVM(nullptr)
, mVMD2(false)
, mPSLT(nullptr)
, mPSCL(nullptr)
, mPCOL(nullptr)
, mPOFS(nullptr)
, mOFST(nullptr)
, mAPSO(false)
#if VERSION >= VERSION_GM8P_00
, mF60H(false)
, mSVBD(false)
, mNDTT(false)
#endif
, mAP11(false)
#if VERSION >= VERSION_GM8P_00
, mSPS1(false)
#endif
, mAP21(false)
#if VERSION >= VERSION_GM8P_00
, mSPS2(false)
#endif
, mAS11(false)
, mAS12(false)
, mAS13(false)
#if VERSION >= VERSION_GM8P_00
, mEWTR(true)
, mLWTR(true)
, mSWTR(true)
, mFC60(false)
, mTRAT(nullptr)
#else
, mTRAT(nullptr)
, mEWTR(true)
, mLWTR(true)
, mSWTR(true)
#endif
, mPJFX(-1)
, mRNGE(nullptr)
, mFOFF(nullptr) {
}

CWeaponDescription::~CWeaponDescription() {
  delete mIVEC;
  delete mIORN;
  delete mPSVM;
  delete mPSOV;
  delete mPSLT;
  delete mPSCL;
  delete mPCOL;
  delete mPOFS;
  delete mOFST;
  delete mTRAT;
  delete mRNGE;
  delete mFOFF;
}
