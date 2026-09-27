#ifndef _CGRAPPLEPARAMETERS
#define _CGRAPPLEPARAMETERS

class CGrappleParameters {
  float mA;
  float mB;
  float mC;
  float mD;
  float mE;
  float mF;
  float mG;
  float mH;
  float mI;
  float mJ;
  float mK;
  bool mLockSwingTurn : 1;

public:
  CGrappleParameters(float a, float b, float c, float d, float e, float f, float g, float h,
                     float i, float j, float k, bool lockSwingTurn)
  : mA(a)
  , mB(b)
  , mC(c)
  , mD(d)
  , mE(e)
  , mF(f)
  , mG(g)
  , mH(h)
  , mI(i)
  , mJ(j)
  , mK(k)
  , mLockSwingTurn(lockSwingTurn) {}

  bool GetLockSwingTurn() const { return mLockSwingTurn; }
};

#endif // _CGRAPPLEPARAMETERS
