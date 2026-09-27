#ifndef _CGENDESCRIPTION
#define _CGENDESCRIPTION

#include "Kyoto/CToken.hpp"
#include "types.h"

#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CElectricDescription;
class CModel;
class CSwooshDescription;

class CGenDescription {
public:
  CGenDescription();
  ~CGenDescription();

  CIntElement* mPSLT;
  CIntElement* mPSWT;
  CRealElement* mPSTS;
  CVectorElement* mPOFS;
  CIntElement* mSEED;
  CRealElement* mLENG;
  CRealElement* mWIDT;
  CIntElement* mMAXP;
  CRealElement* mGRTE;
  CColorElement* mCOLR;
  CIntElement* mLTME;
  CEmitterElement* mEMTR;
  bool mLINE : 1;
  bool mFXLL : 1;
  bool mAAPH : 1;
  bool mZBUF : 1;
  bool mSORT : 1;
  bool mLIT_ : 1;
  bool mORNT : 1;
  bool mRSOP : 1;
  bool mMBLR : 1;
  bool mPMAB : 1;
  bool mPMUS : 1;
  bool mPMOO : 1;
  bool mVMD1 : 1;
  bool mVMD2 : 1;
  bool mVMD3 : 1;
  bool mVMD4 : 1;
  bool mCIND : 1;
  bool mOPTS : 1;
  CIntElement* mMBSP;
  CRealElement* mSIZE;
  CRealElement* mROTA;
  CUVElement* mTEXR;
  CUVElement* mTIND;
  rstl::optional_object< TCachedToken< CModel > > mPMDL;
  CVectorElement* mPMOP;
  CVectorElement* mPMRT;
  CVectorElement* mPMSC;
  CColorElement* mPMCL;
  CModVectorElement* mVEL1;
  CModVectorElement* mVEL2;
  CModVectorElement* mVEL3;
  CModVectorElement* mVEL4;
  rstl::optional_object< TCachedToken< CGenDescription > > mICTS;
  CIntElement* mNCSY;
  CIntElement* mCSSD;
  rstl::optional_object< TCachedToken< CGenDescription > > mIDTS;
  CIntElement* mNDSY;
  rstl::optional_object< TCachedToken< CGenDescription > > mIITS;
  CIntElement* mPISY;
  CIntElement* mSISY;
  rstl::single_ptr< CSpawnSystemKeyframeData > mKSSM;
  rstl::optional_object< TCachedToken< CSwooshDescription > > mSSWH;
  CIntElement* mSSSD;
  CVectorElement* mSSPO;
  rstl::optional_object< CToken > mSELC;
  CIntElement* mSESD;
  CVectorElement* mSEPO;
  CIntElement* mLTYP;
  CColorElement* mLCLR;
  CRealElement* mLINT;
  CVectorElement* mLOFF;
  CVectorElement* mLDIR;
  CIntElement* mLFOT;
  CRealElement* mLFOR;
  CRealElement* mLSLA;
  CRealElement* mADV1;
  CRealElement* mADV2;
  CRealElement* mADV3;
  CRealElement* mADV4;
  CRealElement* mADV5;
  CRealElement* mADV6;
  CRealElement* mADV7;
  CRealElement* mADV8;
};
CHECK_SIZEOF(CGenDescription, 0x12c)

#endif // _CGENDESCRIPTION
