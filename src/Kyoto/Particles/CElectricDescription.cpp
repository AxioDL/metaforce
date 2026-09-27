#include "Kyoto/Particles/CElectricDescription.hpp"

CElectricDescription::CElectricDescription()
: mLIFE(nullptr)
, mSLIF(nullptr)
, mGRAT(nullptr)
, mSCNT(nullptr)
, mSSEG(nullptr)
, mCOLR(nullptr)
, mIEMT(nullptr)
, mFEMT(nullptr)
, mAMPL(nullptr)
, mAMPD(nullptr)
, mLWD1(nullptr)
, mLWD2(nullptr)
, mLWD3(nullptr)
, mLCL1(nullptr)
, mLCL2(nullptr)
, mLCL3(nullptr)
, mZERY(false) {}

CElectricDescription::~CElectricDescription() {
  delete mLIFE;
  delete mSLIF;
  delete mGRAT;
  delete mSCNT;
  delete mSSEG;
  delete mCOLR;
  delete mIEMT;
  delete mFEMT;
  delete mAMPL;
  delete mAMPD;
  delete mLWD1;
  delete mLWD2;
  delete mLWD3;
  delete mLCL1;
  delete mLCL2;
  delete mLCL3;
}
