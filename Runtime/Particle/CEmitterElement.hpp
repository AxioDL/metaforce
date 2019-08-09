#pragma once

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Emitter_Elements */

namespace urde {

class CEESimpleEmitter : public CEmitterElement {
  std::unique_ptr<CVectorElement> x4_loc;
  std::unique_ptr<CVectorElement> x8_vec;

public:
  CEESimpleEmitter(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b)
  : x4_loc(std::move(a)), x8_vec(std::move(b)) {}
  bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const override;
};

class CVESphere : public CEmitterElement {
  std::unique_ptr<CVectorElement> x4_sphereOrigin;
  std::unique_ptr<CRealElement> x8_sphereRadius;
  std::unique_ptr<CRealElement> xc_velocityMag;

public:
  CVESphere(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CRealElement>&& c)
  : x4_sphereOrigin(std::move(a)), x8_sphereRadius(std::move(b)), xc_velocityMag(std::move(c)) {}
  bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const override;
};

class CVEAngleSphere : public CEmitterElement {
  std::unique_ptr<CVectorElement> x4_sphereOrigin;
  std::unique_ptr<CRealElement> x8_sphereRadius;
  std::unique_ptr<CRealElement> xc_velocityMag;
  std::unique_ptr<CRealElement> x10_angleXBias;
  std::unique_ptr<CRealElement> x14_angleYBias;
  std::unique_ptr<CRealElement> x18_angleXRange;
  std::unique_ptr<CRealElement> x1c_angleYRange;

public:
  CVEAngleSphere(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b,
                 std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d,
                 std::unique_ptr<CRealElement>&& e, std::unique_ptr<CRealElement>&& f,
                 std::unique_ptr<CRealElement>&& g)
  : x4_sphereOrigin(std::move(a))
  , x8_sphereRadius(std::move(b))
  , xc_velocityMag(std::move(c))
  , x10_angleXBias(std::move(d))
  , x14_angleYBias(std::move(e))
  , x18_angleXRange(std::move(f))
  , x1c_angleYRange(std::move(g)) {}
  bool GetValue(int frame, zeus::CVector3f& pPos, zeus::CVector3f& pVel) const override;
};

} // namespace urde
