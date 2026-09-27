#include "MetroidPrime/Enemies/CBurstFire.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"

#include "rstl/math.hpp"

CBurstFire::CBurstFire(const SBurst** burstDefs, int firstBurstCount)
: mBurstType(-1)
, mAngleIdx(-1)
, mTimeToNextShot(0.f)
, mFirstBurstIdx(0)
, mFirstBurstCounter(firstBurstCount)
, mShouldFire(false)
, mAvoidAccuracy(false)
, mCurBursts(nullptr) {
  while (*burstDefs) {
    mBurstDefs.push_back(*burstDefs);
    ++burstDefs;
  }
}

void CBurstFire::Start(CStateManager& mgr) {
  const SBurst* bursts = mBurstDefs[mBurstType];
  int burstIdx = -1;

  if (mFirstBurstCounter-- > 0) {
    burstIdx = mFirstBurstIdx >= 0 ? mFirstBurstIdx : 0;

  } else {
    int random = mgr.Random()->Range(0, 100);
    int advanceAccum = 0;
    do {
      burstIdx += 1;
      int advanceWeight = bursts[burstIdx].mRandomSelectionWeight;
      if (advanceWeight == 0) {
        advanceAccum = 100;
        burstIdx -= 1;
      }
      advanceAccum += advanceWeight;
    } while (random > advanceAccum);
  }
  mCurBursts = &bursts[burstIdx];
  mAngleIdx = -1;
  mTimeToNextShot = 0.f;
  mShouldFire = false;
}

void CBurstFire::Update(CStateManager& mgr, float dt) {
  mShouldFire = false;
  if (!mCurBursts) {
    return;
  }

  mTimeToNextShot -= dt;
  if (mTimeToNextShot < 0.f) {
    mAngleIdx += 1;
    if (mCurBursts->mShotAngles[mAngleIdx] > 0) {
      mShouldFire = true;
      mTimeToNextShot = mCurBursts->mTimeToNextShot;
      mTimeToNextShot +=
          (mgr.Random()->Float() - 0.5f) * mCurBursts->mTimeToNextShotVariance;
    } else {
      mCurBursts = nullptr;
    }
  }
}

CVector3f CBurstFire::GetError(float xMag, float zMag) const {
  CVector3f result = CVector3f::Zero();

  if (mShouldFire && mCurBursts) {

    int r0 = mCurBursts->mShotAngles[mAngleIdx];
    if (mAvoidAccuracy && (r0 == 4 || r0 == 12)) {
      r0 = mCurBursts->mShotAngles[mAngleIdx > 0 ? mAngleIdx - 1 : mAngleIdx + 1];
    }

    if (r0 > 0) {
      float angle = r0 * (-22.5f * (M_PIF / 180.f));
      result.SetX(CMath::FastCosR(angle) * xMag);
      result.SetZ(CMath::FastSinR(angle) * zMag);
    }
  }

  return result;
}

CVector3f CBurstFire::GetDistanceCompensatedError(float dist, float maxErrDist) const {
  float xErr = GetMaxXError();
  float zErr = GetMaxZError();
  float div = dist / maxErrDist;
  xErr = rstl::min_val(div * xErr, xErr);
  zErr = rstl::min_val(div * zErr, zErr);
  return GetError(xErr, zErr);
}

float CBurstFire::GetMaxXError() const {
  return gpTweakPlayer->mPlayerXYHalfExtent * 3.625f + 0.2f;
}

float CBurstFire::GetMaxZError() const { return gpTweakPlayer->mEyeOffset; }
