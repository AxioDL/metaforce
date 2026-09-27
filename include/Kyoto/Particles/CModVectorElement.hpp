#ifndef _CMODVECTORELEMENT
#define _CMODVECTORELEMENT

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/IElement.hpp"

class CMVEConstant : public CModVectorElement {
  CRealElement* mX;
  CRealElement* mY;
  CRealElement* mZ;

public:
  CMVEConstant(CRealElement* a, CRealElement* b, CRealElement* c);
  ~CMVEConstant() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEFastConstant : public CModVectorElement {
  CVector3f mVal;

public:
  CMVEFastConstant(float a, float b, float c);
  ~CMVEFastConstant() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEGravity : public CModVectorElement {
  CVectorElement* mA;

public:
  CMVEGravity(CVectorElement* a);
  ~CMVEGravity() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEWind : public CModVectorElement {
  CVectorElement* mVelocity;
  CRealElement* mFactor;

public:
  CMVEWind(CVectorElement* velocity, CRealElement* factor);
  ~CMVEWind() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEExplode : public CModVectorElement {
  CRealElement* mA;
  CRealElement* mB;

public:
  CMVEExplode(CRealElement* a, CRealElement* b);
  ~CMVEExplode() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVETimeChain : public CModVectorElement {
  CModVectorElement* mA;
  CModVectorElement* mB;
  CIntElement* mSwFrame;

public:
  CMVETimeChain(CModVectorElement* a, CModVectorElement* b, CIntElement* c);
  ~CMVETimeChain() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEPulse : public CModVectorElement {
  CIntElement* mADuration;
  CIntElement* mBDuration;
  CModVectorElement* mAVal;
  CModVectorElement* mBVal;

public:
  CMVEPulse(CIntElement* a, CIntElement* b, CModVectorElement* c, CModVectorElement* d);
  ~CMVEPulse() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEImplosion : public CModVectorElement {
  CVectorElement* mImplPoint;
  CRealElement* mMagScale;
  CRealElement* mMaxMag;
  CRealElement* mMinMag;
  bool mEnableMinMag;

public:
  CMVEImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e);
  ~CMVEImplosion() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVELinearImplosion : public CModVectorElement {
  CVectorElement* mImplPoint;
  CRealElement* mMagScale;
  CRealElement* mMaxMag;
  CRealElement* mMinMag;
  bool mEnableMinMag;

public:
  CMVELinearImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d, bool e);
  ~CMVELinearImplosion() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEExponentialImplosion : public CModVectorElement {
  CVectorElement* mImplPoint;
  CRealElement* mMagScale;
  CRealElement* mMaxMag;
  CRealElement* mMinMag;
  bool mEnableMinMag;

public:
  CMVEExponentialImplosion(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                           bool e);
  ~CMVEExponentialImplosion() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVESwirl : public CModVectorElement {
  CVectorElement* mHelixPoint;
  CVectorElement* mCurveBinormal;
  CRealElement* mFilterGain;
  CRealElement* mTangentialVelocity;

public:
  CMVESwirl(CVectorElement* a, CVectorElement* b, CRealElement* c, CRealElement* d);
  ~CMVESwirl() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVEBounce : public CModVectorElement {
  CVectorElement* mPlanePoint;
  CVectorElement* mPlaneNormal;
  CRealElement* mFriction;
  CRealElement* mRestitution;
  bool mPlanePrecomputed;
  bool mDieOnPenetrate;
  mutable CVector3f mPlaneValidatedNormal;
  mutable float mPlaneD;

public:
  CMVEBounce(CVectorElement* planePoint, CVectorElement* planeNormal, CRealElement* friction,
             CRealElement* restitution, const bool e);
  ~CMVEBounce() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

class CMVESetPosition : public CModVectorElement {
  CVectorElement* mA;

public:
  CMVESetPosition(CVectorElement* a);
  ~CMVESetPosition() override;
  bool GetValue(int frame, CVector3f& pVel, CVector3f& pPos) const override;
};

#endif // _CMODVECTORELEMENT
