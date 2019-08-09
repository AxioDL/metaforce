#pragma once

#include "IElement.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Vector_Elements */

namespace urde {

class CVEKeyframeEmitter : public CVectorElement {
  u32 x4_percent;
  u32 x8_unk1;
  bool xc_loop;
  bool xd_unk2;
  u32 x10_loopEnd;
  u32 x14_loopStart;
  std::vector<zeus::CVector3f> x18_keys;

public:
  CVEKeyframeEmitter(CInputStream& in);
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVECone : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_direction;
  std::unique_ptr<CRealElement> x8_magnitude;
  zeus::CVector3f xc_xVec;
  zeus::CVector3f x18_yVec;

public:
  CVECone(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b);
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVETimeChain : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_a;
  std::unique_ptr<CVectorElement> x8_b;
  std::unique_ptr<CIntElement> xc_swFrame;

public:
  CVETimeChain(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b,
               std::unique_ptr<CIntElement>&& c)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_swFrame(std::move(c)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEAngleCone : public CVectorElement {
  std::unique_ptr<CRealElement> x4_angleXConstant;
  std::unique_ptr<CRealElement> x8_angleYConstant;
  std::unique_ptr<CRealElement> xc_angleXRange;
  std::unique_ptr<CRealElement> x10_angleYRange;
  std::unique_ptr<CRealElement> x14_magnitude;

public:
  CVEAngleCone(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CRealElement>&& c,
               std::unique_ptr<CRealElement>&& d, std::unique_ptr<CRealElement>&& e)
  : x4_angleXConstant(std::move(a))
  , x8_angleYConstant(std::move(b))
  , xc_angleXRange(std::move(c))
  , x10_angleYRange(std::move(d))
  , x14_magnitude(std::move(e)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEAdd : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_a;
  std::unique_ptr<CVectorElement> x8_b;

public:
  CVEAdd(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVECircleCluster : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_a;
  zeus::CVector3f x8_xVec;
  zeus::CVector3f x14_yVec;
  float x20_deltaAngle;
  std::unique_ptr<CRealElement> x24_magnitude;

public:
  CVECircleCluster(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b,
                   std::unique_ptr<CIntElement>&& c, std::unique_ptr<CRealElement>&& d);
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEConstant : public CVectorElement {
  std::unique_ptr<CRealElement> x4_a;
  std::unique_ptr<CRealElement> x8_b;
  std::unique_ptr<CRealElement> xc_c;

public:
  CVEConstant(std::unique_ptr<CRealElement>&& a, std::unique_ptr<CRealElement>&& b, std::unique_ptr<CRealElement>&& c)
  : x4_a(std::move(a)), x8_b(std::move(b)), xc_c(std::move(c)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEFastConstant : public CVectorElement {
  zeus::CVector3f x4_val;

public:
  CVEFastConstant(float a, float b, float c) : x4_val(a, b, c) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
  bool IsFastConstant() const override { return true; }
};

class CVECircle : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_direction;
  zeus::CVector3f x8_xVec;
  zeus::CVector3f x14_yVec;
  std::unique_ptr<CRealElement> x20_angleConstant;
  std::unique_ptr<CRealElement> x24_angleLinear;
  std::unique_ptr<CRealElement> x28_radius;

public:
  CVECircle(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b, std::unique_ptr<CRealElement>&& c,
            std::unique_ptr<CRealElement>&& d, std::unique_ptr<CRealElement>&& e);
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEMultiply : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_a;
  std::unique_ptr<CVectorElement> x8_b;

public:
  CVEMultiply(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVERealToVector : public CVectorElement {
  std::unique_ptr<CRealElement> x4_a;

public:
  CVERealToVector(std::unique_ptr<CRealElement>&& a) : x4_a(std::move(a)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEPulse : public CVectorElement {
  std::unique_ptr<CIntElement> x4_aDuration;
  std::unique_ptr<CIntElement> x8_bDuration;
  std::unique_ptr<CVectorElement> xc_aVal;
  std::unique_ptr<CVectorElement> x10_bVal;

public:
  CVEPulse(std::unique_ptr<CIntElement>&& a, std::unique_ptr<CIntElement>&& b, std::unique_ptr<CVectorElement>&& c,
           std::unique_ptr<CVectorElement>&& d)
  : x4_aDuration(std::move(a)), x8_bDuration(std::move(b)), xc_aVal(std::move(c)), x10_bVal(std::move(d)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleVelocity : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleColor : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleLocation : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleSystemOrientationFront : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleSystemOrientationUp : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleSystemOrientationRight : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEParticleSystemTranslation : public CVectorElement {
public:
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVESubtract : public CVectorElement {
  std::unique_ptr<CVectorElement> x4_a;
  std::unique_ptr<CVectorElement> x8_b;

public:
  CVESubtract(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b)
  : x4_a(std::move(a)), x8_b(std::move(b)) {}
  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

class CVEColorToVector : public CVectorElement {
  std::unique_ptr<CColorElement> x4_a;

public:
  CVEColorToVector(std::unique_ptr<CColorElement>&& a) : x4_a(std::move(a)) {}

  bool GetValue(int frame, zeus::CVector3f& valOut) const override;
};

} // namespace urde
