#ifndef _CEMITTERELEMENT
#define _CEMITTERELEMENT

#include "Kyoto/Particles/IElement.hpp"

class CEESimpleEmitter : public CEmitterElement {
public:
  CEESimpleEmitter(CVectorElement* pos, CVectorElement* vel);
  ~CEESimpleEmitter();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mPos;
  CVectorElement* mVel;
};

class CVESphere : public CEmitterElement {
public:
  CVESphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag);
  ~CVESphere();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mSphereOrigin;
  CRealElement* mSphereRadius;
  CRealElement* mVelocityMag;
};

class CVEAngleSphere : public CEmitterElement {
public:
  CVEAngleSphere(CVectorElement* origin, CRealElement* radius, CRealElement* velocityMag,
                 CRealElement* angleXBias, CRealElement* angleYBias, CRealElement* angleXRange,
                 CRealElement* angleYRange);
  ~CVEAngleSphere();

  bool GetValue(int frame, CVector3f& pPos, CVector3f& pVel) const;

private:
  CVectorElement* mSphereOrigin;
  CRealElement* mSphereRadius;
  CRealElement* mVelocityMag;
  CRealElement* mAngleXBias;
  CRealElement* mAngleYBias;
  CRealElement* mAngleXRange;
  CRealElement* mAngleYRange;
};

#endif // _CEMITTERELEMENT
