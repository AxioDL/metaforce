#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"

static IElement** element_from_desc(CGenDescription* gen, int idx) {
  return reinterpret_cast< IElement** >(reinterpret_cast< uchar* >(gen) + idx);
}

static const size_t skGenOffsets[] = {
    offsetof(CGenDescription, mPSLT),   offsetof(CGenDescription, mPSWT),
    offsetof(CGenDescription, mPSTS),   offsetof(CGenDescription, mPOFS),
    offsetof(CGenDescription, mSEED),  offsetof(CGenDescription, mLENG),
    offsetof(CGenDescription, mWIDT),  offsetof(CGenDescription, mMAXP),
    offsetof(CGenDescription, mGRTE),  offsetof(CGenDescription, mCOLR),
    offsetof(CGenDescription, mLTME),  offsetof(CGenDescription, mEMTR),
    offsetof(CGenDescription, mMBSP),  offsetof(CGenDescription, mSIZE),
    offsetof(CGenDescription, mROTA),  offsetof(CGenDescription, mTEXR),
    offsetof(CGenDescription, mTIND),  offsetof(CGenDescription, mPMOP),
    offsetof(CGenDescription, mPMRT),  offsetof(CGenDescription, mPMSC),
    offsetof(CGenDescription, mPMCL),  offsetof(CGenDescription, mVEL1),
    offsetof(CGenDescription, mVEL2),  offsetof(CGenDescription, mVEL3),
    offsetof(CGenDescription, mVEL4),  offsetof(CGenDescription, mNCSY),
    offsetof(CGenDescription, mCSSD),  offsetof(CGenDescription, mNDSY),
    offsetof(CGenDescription, mPISY),  offsetof(CGenDescription, mSISY),
    offsetof(CGenDescription, mSSSD),  offsetof(CGenDescription, mSSPO),
    offsetof(CGenDescription, mSESD),  offsetof(CGenDescription, mSEPO),
    offsetof(CGenDescription, mLTYP),  offsetof(CGenDescription, mLCLR),
    offsetof(CGenDescription, mLINT),  offsetof(CGenDescription, mLOFF),
    offsetof(CGenDescription, mLDIR),  offsetof(CGenDescription, mLFOT),
    offsetof(CGenDescription, mLFOR), offsetof(CGenDescription, mLSLA),
    offsetof(CGenDescription, mADV1), offsetof(CGenDescription, mADV2),
    offsetof(CGenDescription, mADV3), offsetof(CGenDescription, mADV4),
    offsetof(CGenDescription, mADV5), offsetof(CGenDescription, mADV6),
    offsetof(CGenDescription, mADV7), offsetof(CGenDescription, mADV8),
};

CGenDescription::CGenDescription()
: mPSLT(nullptr)
, mPSWT(nullptr)
, mPSTS(nullptr)
, mPOFS(nullptr)
, mSEED(nullptr)
, mLENG(nullptr)
, mWIDT(nullptr)
, mMAXP(nullptr)
, mGRTE(nullptr)
, mCOLR(nullptr)
, mLTME(nullptr)
, mEMTR(nullptr)
, mLINE(false)
, mFXLL(false)
, mAAPH(false)
, mZBUF(false)
, mSORT(false)
, mLIT_(false)
, mORNT(false)
, mRSOP(false)
, mMBLR(false)
, mPMAB(false)
, mPMUS(false)
, mPMOO(true)
, mVMD1(false)
, mVMD2(false)
, mVMD3(false)
, mVMD4(false)
, mCIND(false)
, mOPTS(false)
, mMBSP(nullptr)
, mSIZE(nullptr)
, mROTA(nullptr)
, mTEXR(nullptr)
, mTIND(nullptr)
, mPMOP(nullptr)
, mPMRT(nullptr)
, mPMSC(nullptr)
, mPMCL(nullptr)
, mVEL1(nullptr)
, mVEL2(nullptr)
, mVEL3(nullptr)
, mVEL4(nullptr)
, mNCSY(nullptr)
, mCSSD(nullptr)
, mNDSY(nullptr)
, mPISY(nullptr)
, mSISY(nullptr)
, mSSSD(nullptr)
, mSSPO(nullptr)
, mSESD(nullptr)
, mSEPO(nullptr)
, mLTYP(nullptr)
, mLCLR(nullptr)
, mLINT(nullptr)
, mLOFF(nullptr)
, mLDIR(nullptr)
, mLFOT(nullptr)
, mLFOR(nullptr)
, mLSLA(nullptr)
, mADV1(nullptr)
, mADV2(nullptr)
, mADV3(nullptr)
, mADV4(nullptr)
, mADV5(nullptr)
, mADV6(nullptr)
, mADV7(nullptr)
, mADV8(nullptr) {}

CGenDescription::~CGenDescription() {
  for (int i = 0; i < ARRAY_SIZE(skGenOffsets); ++i) {
    delete *element_from_desc(this, skGenOffsets[i]);
  }
}
