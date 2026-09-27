#ifndef _CSWOOSHDESCRIPTION
#define _CSWOOSHDESCRIPTION

class CColorElement;
class CIntElement;
class CModVectorElement;
class CRealElement;
class CUVElement;
class CVectorElement;
class CSwooshDescription {
public:
  CSwooshDescription();
  ~CSwooshDescription();

  CIntElement* mPSLT;
  CRealElement* mTIME;
  CRealElement* mLRAD;
  CRealElement* mRRAD;
  CIntElement* mLENG;
  CColorElement* mCOLR;
  CIntElement* mSIDE;
  CRealElement* mIROT;
  CRealElement* mROTM;
  CVectorElement* mPOFS;
  CVectorElement* mIVEL;
  CVectorElement* mNPOS;
  CModVectorElement* mVELM;
  CModVectorElement* mVLM2;
  CIntElement* mSPLN;
  CUVElement* mTEXR;
  CIntElement* mTSPN;
  bool mLLRD : 1;
  bool mCROS : 1;
  bool mVLS1 : 1;
  bool mVLS2 : 1;
  bool mSROT : 1;
  bool mWIRE : 1;
  bool mTEXW : 1;
  bool mAALP : 1;
  bool mZBUF : 1;
  bool mORNT : 1;
  bool mCRND : 1;
};

#endif // _CSWOOSHDESCRIPTION
