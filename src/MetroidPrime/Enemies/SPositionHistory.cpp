#include "MetroidPrime/Enemies/SPositionHistory.hpp"

SPositionHistory::SPositionHistory(float mag) : mMagSquared(mag * mag) {}

void SPositionHistory::AddValue(CVector3f pos) {
  if (mValues.size() >= 16) {
    return;
  }
  if (mValues.empty()) {
    mValues.push_back(pos);
    return;
  }
  CVector3f diff = pos - mValues.back();
  if (diff.MagSquared() > mMagSquared) {
    mValues.push_back(pos);
  }
}

CVector3f SPositionHistory::GetValue(CVector3f pos, CVector3f face) {
  CVector3f result = CVector3f::Zero();

  while (!mValues.empty()) {
    const CVector3f v = mValues.back() - pos;
    if (CVector3f::Dot(v, face) > 0.f && v.IsMagnitudeSafe()) {
      result = v.AsNormalized();
      break;
    }
    mValues.pop_back();
  }

  return result;
}
