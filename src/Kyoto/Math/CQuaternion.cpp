#include "Kyoto/Math/CQuaternion.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/math.hpp"

const CQuaternion CQuaternion::sNoRotation = CQuaternion(1.f, CVector3f(0.f, 0.f, 0.f));
static const CUnitVector3f XAxis(1.f, 0.f, 0.f);
static const CUnitVector3f YAxis(0.f, 1.f, 0.f);
static const CUnitVector3f ZAxis(0.f, 0.f, 1.f);

CQuaternion::CQuaternion(CInputStream& in) : w(in.ReadFloat()), imaginary(in) {}

CQuaternion CQuaternion::FromMatrixRows(const CVector3f& row0, const CVector3f& row1,
                                        const CVector3f& row2) {
  const float trace = 1.f + row0.GetX() + row1.GetY() + row2.GetZ();
  if (trace > 1.f) {
    const float qw = CMath::SqrtF(trace) / 2.f;
    const float scale = 0.25f / qw;
    const float qx = scale * (row2.GetY() - row1.GetZ());
    const float qy = scale * (row0.GetZ() - row2.GetX());
    const float qz = scale * (row1.GetX() - row0.GetY());
    return CQuaternion(qw, qx, qy, qz);
  }

  int axis;
  if (row1.GetY() > row0.GetX()) {
    axis = row2.GetZ() > row1.GetY() ? 2 : 1;
  } else {
    axis = row2.GetZ() > row0.GetX() ? 2 : 0;
  }

  if (axis == 0) {
    const float size = 2.f * CMath::SqrtF(row0.GetX() - row1.GetY() - row2.GetZ() + 1.f);
    const float scale = 1.f / size;
    const float qw = scale * (row2.GetY() - row1.GetZ());
    const float qx = size / 4.f;
    const float qy = scale * (row0.GetY() + row1.GetX());
    const float qz = scale * (row0.GetZ() + row2.GetX());
    return CQuaternion(qw, qx, qy, qz);
  } else if (axis == 1) {
    const float size = 2.f * CMath::SqrtF(row1.GetY() - row2.GetZ() - row0.GetX() + 1.f);
    const float scale = 1.f / size;
    const float qw = scale * (row0.GetZ() - row2.GetX());
    const float qy = size / 4.f;
    const float qz = scale * (row1.GetZ() + row2.GetY());
    const float qx = scale * (row1.GetX() + row0.GetY());
    return CQuaternion(qw, qx, qy, qz);
  } else {
    const float size = 2.f * CMath::SqrtF(row2.GetZ() - row0.GetX() - row1.GetY() + 1.f);
    const float scale = 1.f / size;
    const float qw = scale * (row1.GetX() - row0.GetY());
    const float qx = scale * (row2.GetX() + row0.GetZ());
    const float qy = scale * (row2.GetY() + row1.GetZ());
    const float qz = size / 4.f;
    return CQuaternion(qw, qx, qy, qz);
  }
}

CQuaternion CQuaternion::FromMatrix(const CMatrix3f& matrix) {
  const CQuaternion result =
      FromMatrixRows(matrix.GetRow(kDX), matrix.GetRow(kDY), matrix.GetRow(kDZ));
  return result;
}

CQuaternion CQuaternion::FromMatrix(const CTransform4f& matrix) {
  const CQuaternion result =
      FromMatrixRows(matrix.GetRow(kDX), matrix.GetRow(kDY), matrix.GetRow(kDZ));
  return result;
}

CMatrix3f CQuaternion::BuildTransform() const {
  const float x2 = 2.f * AxisX();
  const float y2 = 2.f * AxisY();
  const float z2 = 2.f * AxisZ();
  const float wx = w * x2;
  const float wy = w * y2;
  const float wz = w * z2;
  const float xy = x2 * AxisY();
  const float xz = x2 * AxisZ();
  const float yz = y2 * AxisZ();
  const float xx = x2 * AxisX();
  const float yy = y2 * AxisY();
  const float zz = z2 * AxisZ();
  return CMatrix3f(1.f - yy - zz, xy - wz, xz + wy, xy + wz, 1.f - xx - zz, yz - wx, xz - wy,
                   yz + wx, 1.f - xx - yy);
}

CTransform4f CQuaternion::BuildTransform4f() const {
  const float x2 = 2.f * AxisX();
  const float y2 = 2.f * AxisY();
  const float z2 = 2.f * AxisZ();
  const float wx = w * x2;
  const float wy = w * y2;
  const float wz = w * z2;
  const float xy = x2 * AxisY();
  const float xz = x2 * AxisZ();
  const float yz = y2 * AxisZ();
  const float xx = x2 * AxisX();
  const float yy = y2 * AxisY();
  const float zz = z2 * AxisZ();
  return CTransform4f(1.f - yy - zz, xy - wz, xz + wy, 0.f, xy + wz, 1.f - xx - zz, yz - wx, 0.f,
                      xz - wy, yz + wx, 1.f - xx - yy, 0.f);
}

CTransform4f CQuaternion::BuildTransform4f(const CVector3f& translation) const {
  const float x2 = 2.f * AxisX();
  const float y2 = 2.f * AxisY();
  const float z2 = 2.f * AxisZ();
  const float wx = w * x2;
  const float wy = w * y2;
  const float wz = w * z2;
  const float xy = x2 * AxisY();
  const float xz = x2 * AxisZ();
  const float yz = y2 * AxisZ();
  const float xx = x2 * AxisX();
  const float yy = y2 * AxisY();
  const float zz = z2 * AxisZ();
  return CTransform4f(1.f - yy - zz, xy - wz, xz + wy, translation.GetX(), xy + wz, 1.f - xx - zz,
                      yz - wx, translation.GetY(), xz - wy, yz + wx, 1.f - xx - yy,
                      translation.GetZ());
}

CQuaternion CQuaternion::Slerp(const CQuaternion& a, const CQuaternion& b, float t) {
  const float product = Dot(a, b);
  const float dot = product > 1.f ? 1.f : product < -1.f ? -1.f : product;
  const double angle = acos(dot);
  const double sineAngle = sin(angle);
  if (sineAngle == 0.0 && dot > 0.f) {
    return a;
  }

  const float aScale = sin(angle * (1.f - t));
  const float bScale = sin(angle * t);
  const double scale = 1.0 / sineAngle;
  const float vectorScale = scale;
  return CQuaternion(scale * (aScale * a.w + bScale * b.w),
                     vectorScale * (aScale * a.imaginary + bScale * b.imaginary));
}

CQuaternion CQuaternion::ShortestRotationArc(const CVector3f& from, const CVector3f& to) {
  CVector3f fromUnit = from;
  CVector3f toUnit = to;
  if (fromUnit.IsNonZero()) {
    fromUnit.Normalize();
  }
  if (toUnit.IsNonZero()) {
    toUnit.Normalize();
  }
  const CVector3f cross = CVector3f::Cross(fromUnit, toUnit);
  if (cross.MagSquared() < 0.001f) {
    if (CVector3f::Dot(fromUnit, toUnit) > 0.f) {
      return NoRotation();
    }
    if (cross.CanBeNormalized()) {
      return CQuaternion(0.f, cross.AsNormalized());
    }
    return NoRotation();
  }

  const float size =
      CMath::SqrtF((1.f + CMath::Clamp(-1.f, CVector3f::Dot(fromUnit, toUnit), 1.f)) * 2.f);
  return CQuaternion(0.5f * size, (1.f / size) * cross);
}

static inline float normalize_angle(float value) {
  static float skHalf = CRelAngle::FromDegrees(180.f).AsRadians();
  float angle = value;
  if (angle > skHalf) {
    angle -= 2.f * skHalf;
  } else if (angle < -skHalf) {
    angle += 2.f * skHalf;
  }
  return angle;
}

CQuaternion CQuaternion::LookAt(const CUnitVector3f& source, const CUnitVector3f& dest,
                                const CRelAngle& maxAngle) {
  CVector3f destNoZ = dest;
  CVector3f sourceNoZ = source;
  const float maxRadians = maxAngle.AsRadians();
  destNoZ.SetZ(0.f);
  sourceNoZ.SetZ(0.f);
  CQuaternion yaw = NoRotation();
  const float sourceMag = sourceNoZ.MagSquared();
  const float destMag = destNoZ.MagSquared();
  CVector3f horizontal(1.f, 0.f, 0.f);
  if (sourceMag > 0.0001f && destMag > 0.0001f) {
    sourceNoZ.Normalize();
    destNoZ.Normalize();
    const float sourceAngle = atan2f(sourceNoZ.GetX(), sourceNoZ.GetY());
    const float destAngle = atan2f(destNoZ.GetX(), destNoZ.GetY());
    float angle = normalize_angle(destAngle - sourceAngle);
    angle = rstl::min_val(angle, maxRadians);
    angle = rstl::max_val(angle, -maxRadians);
    yaw = ZRotation(CRelAngle::FromRadians(-angle));
    horizontal = (yaw * ScalarVector(0.f, sourceNoZ) * yaw.BuildInverted()).GetVector();
  } else if (sourceMag > 0.0001f) {
    horizontal = sourceNoZ.AsNormalized();
  } else if (destMag > 0.0001f) {
    horizontal = destNoZ.AsNormalized();
  } else {
    return NoRotation();
  }

  const CVector3f axis = CVector3f::Cross(horizontal, CVector3f(0.f, 0.f, 1.f));
  float angle = normalize_angle(acosf(dest.GetZ()) - acosf(source.GetZ()));
  angle = rstl::min_val(angle, maxRadians);
  angle = rstl::max_val(angle, -maxRadians);
  const CQuaternion pitch =
      AxisAngle(CUnitVector3f(axis, CUnitVector3f::kN_No), CRelAngle::FromRadians(-angle));
  return pitch * yaw;
}

CQuaternion CQuaternion::SlerpLocal(const CQuaternion& from, const CQuaternion& to, float t) {
  return Dot(from, to) >= 0.f ? Slerp(from, to, t) : Slerp(from, to.BuildEquivalent(), t);
}

CRelAngle CQuaternion::AngleFrom(const CQuaternion& other) const {
  return CRelAngle::FromRadians(
      CMath::ArcCosineR(rstl::min_val(rstl::max_val(Dot(*this, other), -1.f), 1.f)));
}

CQuaternion CQuaternion::BuildEquivalent() const {
  const double angle = 2.0 * acos(rstl::max_val(rstl::min_val(GetScalar(), 1.f), -1.f));
  const double equivalentAngle = angle + 2.0 * M_PI;
  if (close_enough(angle, 0.0, 1.e-7)) {
    return CQuaternion(-1.f, CVector3f::Zero());
  }
  const CUnitVector3f axis(imaginary);
  return AxisAngle(axis, CRelAngle::FromRadians(equivalentAngle));
}

CQuaternion CQuaternion::BuildNormalized() const {
  const float scale = CMath::InvSqrtF(w * w + imaginary.MagSquared());
  return CQuaternion(scale * w, scale * imaginary);
}

CQuaternion CQuaternion::AxisAngle(const CUnitVector3f& axis, const CRelAngle& angle) {
  double w = cos(angle.AsRadians() / 2.f);
  CVector3f vec = axis * sine(angle / 2.f);
  return CQuaternion(w, vec);
}

bool CQuaternion::IsValidQuaternion(float epsilon) const {
  const float error = GetVector().MagSquared() + GetScalar() * GetScalar() - 1.f;
  return (error >= 0.f ? error : -error) < epsilon;
}

CVector3f CQuaternion::Transform(const CVector3f& vector) const {
  const float scalar = -CVector3f::Dot(imaginary, vector);
  const CVector3f rotated(w * vector.GetX() + AxisY() * vector.GetZ() - vector.GetY() * AxisZ(),
                          w * vector.GetY() + AxisZ() * vector.GetX() - vector.GetZ() * AxisX(),
                          w * vector.GetZ() + AxisX() * vector.GetY() - vector.GetX() * AxisY());
  return CVector3f(
      w * rotated.GetX() - scalar * AxisX() - rotated.GetY() * AxisZ() + AxisY() * rotated.GetZ(),
      w * rotated.GetY() - scalar * AxisY() - rotated.GetZ() * AxisX() + AxisZ() * rotated.GetX(),
      w * rotated.GetZ() - scalar * AxisZ() - rotated.GetX() * AxisY() + AxisX() * rotated.GetY());
}

CQuaternion CQuaternion::operator*(const CQuaternion& rhs) const {
  const CVector3f& leftVector = GetVector();
  const CVector3f& rightVector = rhs.GetVector();
  const float leftScalar = GetScalar();
  const float rightScalar = rhs.GetScalar();
  const float scalar = leftScalar * rightScalar - CVector3f::Dot(leftVector, rightVector);
  return CQuaternion(scalar, leftScalar * rightVector + rightScalar * leftVector +
                                 CVector3f::Cross(leftVector, rightVector));
}

CQuaternion CQuaternion::XRotation(const CRelAngle& angle) { return AxisAngle(XAxis, angle); }

CQuaternion CQuaternion::YRotation(const CRelAngle& angle) { return AxisAngle(YAxis, angle); }

CQuaternion CQuaternion::ZRotation(const CRelAngle& angle) { return AxisAngle(ZAxis, angle); }

CQuaternion CQuaternion::ClampedRotateTo(const CVector3f& from, const CVector3f& to,
                                         const CRelAngle& angle) {
  const CQuaternion arc = ShortestRotationArc(from, to);
  const float radians = angle.AsRadians();
  if (radians >= 2.f * acosf(arc.GetScalar())) {
    return arc;
  }
  return AxisAngle(CUnitVector3f(arc.GetVector()), angle);
}
