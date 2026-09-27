#ifndef _CDECALDESCRIPTION
#define _CDECALDESCRIPTION

#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CDecalDescription {
public:
  struct SQuadDescr {
    rstl::single_ptr< CIntElement > mLFT;
    rstl::single_ptr< CRealElement > mSZE;
    rstl::single_ptr< CRealElement > mROT;
    rstl::single_ptr< CVectorElement > mOFF;
    rstl::single_ptr< CColorElement > mCLR;
    rstl::single_ptr< CUVElement > mTEX;
    bool mADD;
    SQuadDescr();
  };

  CDecalDescription();

  // private:
  SQuadDescr mQuad1;
  SQuadDescr mQuad2;
  rstl::optional_object< TLockedToken< CModel > > mDMDL;
  rstl::single_ptr< CIntElement > mDLFT;
  rstl::single_ptr< CVectorElement > mDMOP;
  rstl::single_ptr< CVectorElement > mDMRT;
  rstl::single_ptr< CVectorElement > mDMSC;
  rstl::single_ptr< CColorElement > mDMCL;
  bool mDMAB : 1;
  bool mDMOO : 1;
};

CHECK_SIZEOF(CDecalDescription, 0x60);

#endif // _CDECALDESCRIPTION
