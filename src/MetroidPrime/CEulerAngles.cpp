#include "MetroidPrime/CEulerAngles.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CEulerAngles CEulerAngles::sIdentity(0.f, 0.f, 0.f);

CEulerAngles CEulerAngles::FromTransform(const CTransform4f& xf) {
  const CMatrix3f& mtx = CMatrix3f::FromTransform(xf);
  const float sq = sqrt(mtx.Get11() * mtx.Get11() + mtx.Get01() * mtx.Get01());

  if (!close_enough(sq, 0.f)) {
    double roll;
    double pitch;
    double yaw;
    yaw = -atan2(mtx.Get01(), mtx.Get11());
    pitch = -atan2(mtx.Get20(), mtx.Get22());
    roll = -atan2(-mtx.Get21(), sq);
    return CEulerAngles(roll, pitch, yaw);
  }

  double pitch = -atan2(-mtx.Get02(), mtx.Get00());
  double roll = -atan2(-mtx.Get21(), sq);
  return CEulerAngles(roll, pitch, 0.f);
}


CEulerAngles CEulerAngles::FromQuaternion(const CQuaternion& quat) {
  float magnitudeSquared = quat.GetVector().GetX() * quat.GetVector().GetX() +
                           quat.GetVector().GetY() * quat.GetVector().GetY() +
                           quat.GetVector().GetZ() * quat.GetVector().GetZ() +
                           quat.GetScalar() * quat.GetScalar();
  float scale = magnitudeSquared > 0.f ? 2.f / magnitudeSquared : 0.f;

  float sx = scale * quat.GetVector().GetX();
  float sy = scale * quat.GetVector().GetY();
  float sz = scale * quat.GetVector().GetZ();

  float zz = sz * quat.GetVector().GetZ();
  float xx = sx * quat.GetVector().GetX();
  float wz = sz * quat.GetScalar();
  float yy = sy * quat.GetVector().GetY();
  float xy = sy * quat.GetVector().GetX();
  float wy = sy * quat.GetScalar();
  float xz = sz * quat.GetVector().GetX();
  float yz = sz * quat.GetVector().GetY();
  float wx = sx * quat.GetScalar();

  CMatrix3f mtx(1.f - (yy + zz), xy - wz, xz + wy, xy + wz, 1.f - (xx + zz), yz - wx, xz - wy,
                yz + wx, 1.f - (xx + yy));
  float sq = sqrt(mtx.Get11() * mtx.Get11() + mtx.Get01() * mtx.Get01());

  if (!close_enough(sq, 0.f)) {
    double yaw = -atan2(mtx.Get01(), mtx.Get11());
    double pitch = -atan2(mtx.Get20(), mtx.Get22());
    double roll = -atan2(-mtx.Get21(), sq);
    return CEulerAngles(roll, pitch, yaw);
  }

  double pitch = -atan2(-mtx.Get02(), mtx.Get00());
  double roll = -atan2(-mtx.Get21(), sq);
  return CEulerAngles(roll, pitch, 0.f);
}

static float hack() {
  static float hack = 1.f;
  static float hack2 = 0.f;
  static float hack3 = 2.f;
  return hack;
}