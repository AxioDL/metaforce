#include "CVectorElement.hpp"
#include "CParticleGlobals.hpp"
#include "CRandom16.hpp"
#include "CElementGen.hpp"
#include "zeus/Math.hpp"
#include "CGenDescription.hpp"

/* Documentation at: http://www.metroid2002.com/retromodding/wiki/Particle_Script#Vector_Elements */

namespace urde {

CVEKeyframeEmitter::CVEKeyframeEmitter(CInputStream& in) {
  x4_percent = in.readUint32Big();
  x8_unk1 = in.readUint32Big();
  xc_loop = in.readBool();
  xd_unk2 = in.readBool();
  x10_loopEnd = in.readUint32Big();
  x14_loopStart = in.readUint32Big();

  u32 count = in.readUint32Big();
  x18_keys.reserve(count);
  for (u32 i = 0; i < count; ++i)
    x18_keys.push_back(in.readVec3fBig());
}

bool CVEKeyframeEmitter::GetValue(int frame, zeus::CVector3f& valOut) const {
  if (!x4_percent) {
    int emitterTime = CParticleGlobals::g_EmitterTime;
    int calcKey = emitterTime;
    if (xc_loop) {
      if (emitterTime >= x10_loopEnd) {
        int v1 = emitterTime - x14_loopStart;
        int v2 = x10_loopEnd - x14_loopStart;
        calcKey = v1 % v2;
        calcKey += x14_loopStart;
      }
    } else {
      int v1 = x10_loopEnd - 1;
      if (v1 < emitterTime)
        calcKey = v1;
    }
    valOut = x18_keys[calcKey];
  } else {
    int ltPerc = CParticleGlobals::g_ParticleLifetimePercentage;
    float ltPercRem = CParticleGlobals::g_ParticleLifetimePercentageRemainder;
    if (ltPerc == 100)
      valOut = x18_keys[100];
    else
      valOut = ltPercRem * x18_keys[ltPerc + 1] + (1.0f - ltPercRem) * x18_keys[ltPerc];
  }
  return false;
}

CVECone::CVECone(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CRealElement>&& b)
: x4_direction(std::move(a)), x8_magnitude(std::move(b)) {
  zeus::CVector3f av;
  x4_direction->GetValue(0, av);
  zeus::CVector3f avNorm = av.normalized();
  if (avNorm.x() > 0.8f)
    xc_xVec = av.cross(zeus::CVector3f(0.f, 1.f, 0.f));
  else
    xc_xVec = av.cross(zeus::CVector3f(1.f, 0.f, 0.f));
  x18_yVec = avNorm.cross(xc_xVec);
}

bool CVECone::GetValue(int frame, zeus::CVector3f& valOut) const {
  float b;
  x8_magnitude->GetValue(frame, b);
  zeus::CVector3f dir;
  x4_direction->GetValue(frame, dir);
  float b2 = std::min(1.f, b);

  float randX, randY;
  do {
    float rand1 = CRandom16::GetRandomNumber()->Float() - 0.5f;
    randX = 2.f * b2 * rand1;
    float rand2 = CRandom16::GetRandomNumber()->Float() - 0.5f;
    randY = 2.f * b2 * rand2;
  } while (randX * randX + randY * randY > 1.f);

  valOut = xc_xVec * randX + x18_yVec * randY + dir;
  return false;
}

bool CVETimeChain::GetValue(int frame, zeus::CVector3f& valOut) const {
  int v;
  xc_swFrame->GetValue(frame, v);
  if (frame >= v)
    return x8_b->GetValue(frame, valOut);
  else
    return x4_a->GetValue(frame, valOut);
}

bool CVEAngleCone::GetValue(int frame, zeus::CVector3f& valOut) const {
  float xc, yc, xr, yr;
  x4_angleXConstant->GetValue(frame, xc);
  x8_angleYConstant->GetValue(frame, yc);
  xc_angleXRange->GetValue(frame, xr);
  x10_angleYRange->GetValue(frame, yr);

  float xtmp = CRandom16::GetRandomNumber()->Float() * xr;
  float xang = zeus::degToRad(0.5f * xr - xtmp + xc);

  float ytmp = CRandom16::GetRandomNumber()->Float() * yr;
  float yang = zeus::degToRad(0.5f * yr - ytmp + yc);

  float mag;
  x14_magnitude->GetValue(frame, mag);

  /* This takes a +Z vector and rotates it around X and Y axis (like a rotation matrix would) */
  valOut = zeus::CVector3f(std::cos(xang) * -std::sin(yang), std::sin(xang), std::cos(xang) * std::cos(yang)) *
           zeus::CVector3f(mag);
  return false;
}

bool CVEAdd::GetValue(int frame, zeus::CVector3f& valOut) const {
  zeus::CVector3f a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a + b;
  return false;
}

CVECircleCluster::CVECircleCluster(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b,
                                   std::unique_ptr<CIntElement>&& c, std::unique_ptr<CRealElement>&& d)
: x4_a(std::move(a)), x24_magnitude(std::move(d)) {
  int cv;
  c->GetValue(0, cv);
  x20_deltaAngle = zeus::degToRad(360.f / float(cv));

  zeus::CVector3f bv;
  b->GetValue(0, bv);
  bv.normalize();
  if (bv[0] > 0.8f)
    x8_xVec = bv.cross(zeus::CVector3f(0.f, 1.f, 0.f));
  else
    x8_xVec = bv.cross(zeus::CVector3f(1.f, 0.f, 0.f));
  x14_yVec = bv.cross(x8_xVec);
}

bool CVECircleCluster::GetValue(int frame, zeus::CVector3f& valOut) const {
  zeus::CVector3f av;
  x4_a->GetValue(frame, av);

  float curAngle = frame * x20_deltaAngle;
  zeus::CVector3f x = x8_xVec * std::cos(curAngle);
  zeus::CVector3f y = x14_yVec * std::sin(curAngle);
  zeus::CVector3f tv = x + y + av;

  float dv;
  x24_magnitude->GetValue(frame, dv);

  zeus::CVector3f magVec(dv * tv.magnitude());
  zeus::CVector3f rv =
      magVec * zeus::CVector3f(CRandom16::GetRandomNumber()->Float(), CRandom16::GetRandomNumber()->Float(),
                               CRandom16::GetRandomNumber()->Float());

  valOut = tv + rv;
  return false;
}

bool CVEConstant::GetValue(int frame, zeus::CVector3f& valOut) const {
  float a, b, c;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  xc_c->GetValue(frame, c);
  valOut = zeus::CVector3f(a, b, c);
  return false;
}

bool CVEFastConstant::GetValue(int frame, zeus::CVector3f& valOut) const {
  valOut = x4_val;
  return false;
}

CVECircle::CVECircle(std::unique_ptr<CVectorElement>&& a, std::unique_ptr<CVectorElement>&& b,
                     std::unique_ptr<CRealElement>&& c, std::unique_ptr<CRealElement>&& d,
                     std::unique_ptr<CRealElement>&& e)
: x4_direction(std::move(a)), x20_angleConstant(std::move(c)), x24_angleLinear(std::move(d)), x28_radius(std::move(e)) {
  zeus::CVector3f bv;
  b->GetValue(0, bv);
  bv.normalize();
  if (bv[0] > 0.8f)
    x8_xVec = bv.cross(zeus::CVector3f(0.f, 1.f, 0.f));
  else
    x8_xVec = bv.cross(zeus::CVector3f(1.f, 0.f, 0.f));
  x14_yVec = bv.cross(x8_xVec);
}

bool CVECircle::GetValue(int frame, zeus::CVector3f& valOut) const {
  float c, d, e;
  x20_angleConstant->GetValue(frame, c);
  x24_angleLinear->GetValue(frame, d);
  x28_radius->GetValue(frame, e);

  float curAngle = zeus::degToRad(d * frame + c);

  zeus::CVector3f av;
  x4_direction->GetValue(frame, av);

  zeus::CVector3f x = x8_xVec * e * std::cos(curAngle);
  zeus::CVector3f y = x14_yVec * e * std::sin(curAngle);

  valOut = x + y + av;
  return false;
}

bool CVEMultiply::GetValue(int frame, zeus::CVector3f& valOut) const {
  zeus::CVector3f a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a * b;
  return false;
}

bool CVERealToVector::GetValue(int frame, zeus::CVector3f& valOut) const {
  float a;
  x4_a->GetValue(frame, a);
  valOut = zeus::CVector3f(a);
  return false;
}

bool CVEPulse::GetValue(int frame, zeus::CVector3f& valOut) const {
  int a, b;
  x4_aDuration->GetValue(frame, a);
  x8_bDuration->GetValue(frame, b);
  int cv = std::max(1, a + b + 1);

  if (b >= 1) {
    int cv2 = frame % cv;
    if (cv2 >= a)
      x10_bVal->GetValue(frame, valOut);
    else
      xc_aVal->GetValue(frame, valOut);
  } else
    xc_aVal->GetValue(frame, valOut);

  return false;
}

bool CVEParticleVelocity::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  valOut = CElementGen::g_currentParticle->x1c_vel;
  return false;
}

bool CVEParticleColor::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  valOut = CElementGen::g_currentParticle->x10_prevPos;
  return false;
}

bool CVEParticleLocation::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  valOut = CElementGen::g_currentParticle->x4_pos;
  return false;
}

bool CVEParticleSystemOrientationFront::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  zeus::CMatrix4f trans =
      CParticleGlobals::g_currentParticleSystem->x4_system->GetOrientation().toMatrix4f().transposed();
  valOut.assign(trans.m[0].y(), trans.m[1].y(), trans.m[2].y());
  return false;
}

bool CVEParticleSystemOrientationUp::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  zeus::CMatrix4f trans =
      CParticleGlobals::g_currentParticleSystem->x4_system->GetOrientation().toMatrix4f().transposed();
  valOut.assign(trans.m[0].z(), trans.m[1].z(), trans.m[2].z());
  return false;
}

bool CVEParticleSystemOrientationRight::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  zeus::CMatrix4f trans =
      CParticleGlobals::g_currentParticleSystem->x4_system->GetOrientation().toMatrix4f().transposed();
  valOut.assign(trans.m[0].x(), trans.m[1].x(), trans.m[2].x());
  return false;
}

bool CVEParticleSystemTranslation::GetValue(int /*frame*/, zeus::CVector3f& valOut) const {
  valOut = CParticleGlobals::g_currentParticleSystem->x4_system->GetTranslation();
  return false;
}

bool CVESubtract::GetValue(int frame, zeus::CVector3f& valOut) const {
  zeus::CVector3f a, b;
  x4_a->GetValue(frame, a);
  x8_b->GetValue(frame, b);
  valOut = a - b;
  return false;
}

bool CVEColorToVector::GetValue(int frame, zeus::CVector3f& valOut) const {
  zeus::CColor val = {0.0f, 0.0f, 0.0f, 1.0f};
  x4_a->GetValue(frame, val);
  valOut = zeus::CVector3f{val.mSimd};
  return false;
}

} // namespace urde
