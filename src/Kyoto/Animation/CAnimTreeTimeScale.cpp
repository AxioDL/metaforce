#include "Kyoto/Animation/CAnimTreeTimeScale.hpp"

#include "Kyoto/Animation/CBoolPOINode.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CParticlePOINode.hpp"
#include "Kyoto/Animation/CSoundPOINode.hpp"

CAdvancementResults CAnimTreeTimeScale::VAdvanceView(const CCharAnimTime& dt) {
  if (dt.EqualsZero() && dt > CCharAnimTime::ZeroFlat())
    return mChild->AdvanceView(dt);

  CCharAnimTime origAccelTime = mCurAccelTime;
  CCharAnimTime newTime = mCurAccelTime + dt;
  if (newTime < mTargetAccelTime) {
    CCharAnimTime integral = mTimeScale->TimeScaleIntegral(origAccelTime, newTime);
    CAdvancementResults res = mChild->AdvanceView(integral);
    if (res.mRemTime.EqualsZero()) {
      mCurAccelTime = newTime;
      return CAdvancementResults(CCharAnimTime::ZeroFlat(), res.mDeltas);
    } else {
      mCurAccelTime = mTimeScale->FindUpperLimit(origAccelTime, integral - res.mRemTime);
      CCharAnimTime elapsed = mCurAccelTime - origAccelTime;
      CCharAnimTime remaining = dt - elapsed;
      return CAdvancementResults(remaining, res.mDeltas);
    }
  } else {
    CCharAnimTime newDt = mTimeScale->TimeScaleIntegral(origAccelTime, mTargetAccelTime);
    CAdvancementResults res(CCharAnimTime(0.f), CAdvancementDeltas());
    if (newDt.GreaterThanZero())
      res = mChild->AdvanceView(newDt);
    CCharAnimTime remTime = res.mRemTime + (newTime - mTargetAccelTime);
    mCurAccelTime = mTargetAccelTime;
    return CAdvancementResults(remTime, res.mDeltas);
  }
}

CCharAnimTime CAnimTreeTimeScale::VGetTimeRemaining() const {
  CCharAnimTime timeRem = mChild->GetTimeRemaining();
  if (mTargetAccelTime == CCharAnimTime::Infinity()) {
    CCharAnimTime remaining =
        mTimeScale->FindUpperLimit(mCurAccelTime, timeRem) - mCurAccelTime;
    return remaining;
  }
  return GetRealLifeTime(timeRem);
}

CSteadyStateAnimInfo CAnimTreeTimeScale::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo info = mChild->GetSteadyStateAnimInfo();
  CCharAnimTime originalDuration = info.GetDuration();
  if (mTargetAccelTime == CCharAnimTime::Infinity()) {
    const CCharAnimTime duration =
        mTimeScale->FindUpperLimit(CCharAnimTime::ZeroFlat(), originalDuration);
    return CSteadyStateAnimInfo(info.IsLooping(), duration, info.GetOffset());
  } else {
    CCharAnimTime time =
        mCurAccelTime.GreaterThanZero()
            ? mTimeScale->TimeScaleIntegral(CCharAnimTime::ZeroFlat(), mCurAccelTime)
            : CCharAnimTime::ZeroFlat();
    CCharAnimTime remaining = GetTimeRemaining();
    CCharAnimTime duration = mInitialTime + time + remaining;
    return CSteadyStateAnimInfo(info.IsLooping(), duration, info.GetOffset());
  }
}

rstl::ownership_transfer< IAnimReader > CAnimTreeTimeScale::VClone() const {
  return rs_new CAnimTreeTimeScale(Cast(mChild->Clone()), mTimeScale->Clone(),
                                   mCurAccelTime, mTargetAccelTime, mInitialTime, mName);
}

rstl::rc_ptr< CAnimTreeNode > CAnimTreeTimeScale::VGetBestUnblendedChild() const {
  rstl::rc_ptr< CAnimTreeNode > child = mChild->GetBestUnblendedChild();
  if (child)
    return rs_new CAnimTreeTimeScale(Cast(child->Clone()), mTimeScale->Clone(), mCurAccelTime,
                                     mTargetAccelTime, mInitialTime, mName);
  return child;
}

CAnimTreeEffectiveContribution CAnimTreeTimeScale::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution contribution = mChild->GetContributionOfHighestInfluence();
  float weight = contribution.GetContributionWeight();
  rstl::string name = contribution.GetPrimitiveName();
  CSteadyStateAnimInfo info = GetSteadyStateAnimInfo();
  CCharAnimTime time = GetTimeRemaining();
  return CAnimTreeEffectiveContribution(weight, name, info, time,
                                        contribution.GetAnimDatabaseIndex());
}

uint CAnimTreeTimeScale::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                         uint capacity, uint iterator, int unk) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetBoolPOIList(useTime, listOut, capacity, iterator, unk);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[iterator + i].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                          uint capacity, uint iterator, int unk) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetInt32POIList(useTime, listOut, capacity, iterator, unk);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                             uint capacity, uint iterator, int unk) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetParticlePOIList(useTime, listOut, capacity, iterator, unk);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

uint CAnimTreeTimeScale::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                          uint capacity, uint iterator, int unk) const {
  const CCharAnimTime useTime =
      time == CCharAnimTime::Infinity() ? mChild->GetTimeRemaining() : GetRealLifeTime(time);
  const uint ret = mChild->GetSoundPOIList(useTime, listOut, capacity, iterator, unk);
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    for (uint i = 0; i < ret; ++i) {
      CCharAnimTime realTime = GetRealLifeTime(listOut[i].GetTime());
      listOut[i + iterator].SetTime(realTime);
    }
  }
  return ret;
}

bool CAnimTreeTimeScale::VGetBoolPOIState(const char* name) const {
  return mChild->GetBoolPOIState(name);
}
s32 CAnimTreeTimeScale::VGetInt32POIState(const char* name) const {
  return mChild->GetInt32POIState(name);
}
CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(const char* name) const {
  return mChild->GetParticlePOIState(name);
}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > CAnimTreeTimeScale::VSimplified() {
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > simp = mChild->Simplified();
  if (simp)
    return rstl::ownership_transfer< IAnimReader >(
        rs_new CAnimTreeTimeScale(Cast(*simp), mTimeScale->Clone(), mCurAccelTime,
                                  mTargetAccelTime, mInitialTime, mName));
  if (mCurAccelTime == mTargetAccelTime)
    return mChild->Clone();
  return rstl::optional_object_null();
}

void CAnimTreeTimeScale::VSetPhase(float phase) { mChild->SetPhase(phase); }

CCharAnimTime CAnimTreeTimeScale::GetRealLifeTime(const CCharAnimTime& time) const {
  CCharAnimTime timeRem = mChild->GetTimeRemaining();
  CCharAnimTime ret(rstl::min_val(time.GetSeconds(), timeRem.GetSeconds()));
  if (mTargetAccelTime > CCharAnimTime::ZeroFlat()) {
    CCharAnimTime accelRemaining = mTargetAccelTime - mCurAccelTime;
    if (ret < accelRemaining) {
      return mTimeScale->TimeScaleIntegral(mCurAccelTime, mCurAccelTime + ret);
    } else {
      CCharAnimTime integral =
          mTimeScale->TimeScaleIntegral(mCurAccelTime, mTargetAccelTime);
      if (integral > ret) {
        CCharAnimTime upper = mTimeScale->FindUpperLimit(mCurAccelTime, ret);
        return upper - mCurAccelTime;
      } else {
        return integral + (ret - integral);
      }
    }
  }
  return ret;
}

rstl::string CAnimTreeTimeScale::CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& node,
                                                     float scaleA, const CCharAnimTime& time,
                                                     float scaleB) {
  return rstl::string("");
}
