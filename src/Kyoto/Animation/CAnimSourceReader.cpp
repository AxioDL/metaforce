#include "Kyoto/Animation/CAnimSourceReader.hpp"

#include "Kyoto/Animation/CCharAnimTime.hpp"

CAnimSourceReader::CAnimSourceReader(const TSubAnimTypeToken< CAnimSource >& source,
                                     const CCharAnimTime& time)
: CAnimSourceReaderBase(rs_new CAnimSourceInfo(source))
, mSource(source)
, mSteadyStateInfo(mSource->GetSteadyStateAnimInfo(time)) {
  PostConstruct(time);
}

CAdvancementResults CAnimSourceReader::VAdvanceView(const CCharAnimTime& dt) {
  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime == duration) {
    mCurTime = CCharAnimTime::ZeroFlat();
    mPassedBoolCount = 0;
    mPassedIntCount = 0;
    mPassedParticleCount = 0;
    mPassedSoundCount = 0;
    return CAdvancementResults(dt, CAdvancementDeltas());
  }
  if (dt.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }

  mCurTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime > duration) {
    remainingTime = mCurTime - duration;
    mCurTime = duration;
  }
  if (mSource->HasPOIData()) {
    UpdatePOIStates();
  }

  const CSegId root = CSegId::Root();
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, mCurTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, mCurTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return CAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));
}

CCharAnimTime CAnimSourceReader::VGetTimeRemaining() const {
  return mSource->GetAnimationDuration() - mCurTime;
}

CSteadyStateAnimInfo CAnimSourceReader::VGetSteadyStateAnimInfo() const {
  return mSteadyStateInfo;
}

bool CAnimSourceReader::VHasOffset(const CSegId& seg) const { return mSource->HasOffset(seg); }

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg) const {
  return mSource->GetOffset(seg, mCurTime);
}

CVector3f CAnimSourceReader::VGetOffset(const CSegId& seg, const CCharAnimTime& time) const {
  return mSource->GetOffset(seg, time);
}

CQuaternion CAnimSourceReader::VGetRotation(const CSegId& seg) const {
  return mSource->GetRotation(seg, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list,
                                            CSegStatementSet& setOut) const {
  mSource->GetSegStatementSet(list, setOut, mCurTime);
}

void CAnimSourceReader::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                            const CCharAnimTime& time) const {
  mSource->GetSegStatementSet(list, setOut, time);
}

rstl::ownership_transfer< IAnimReader > CAnimSourceReader::VClone() const {
  return rstl::ownership_transfer< IAnimReader >(
      rs_new CAnimSourceReader(mSource, mCurTime, mSteadyStateInfo, mPassedBoolCount,
                               mPassedIntCount, mPassedParticleCount, mPassedSoundCount,
                               mBoolStates, mInt32States, mParticleStates));
}

CAdvancementResults CAnimSourceReader::VReverseView(const CCharAnimTime& dt) {
  const CCharAnimTime previousTime = mCurTime;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime.EqualsZero()) {
    mCurTime = duration;
    return CAdvancementResults(dt, CAdvancementDeltas());
  }
  if (dt.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }

  mCurTime -= dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (mCurTime < CCharAnimTime()) {
    remainingTime = CCharAnimTime() - mCurTime;
    mCurTime = CCharAnimTime();
  }

  const CSegId root = CSegId::Root();
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, mCurTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  CVector3f offset(0.f, 0.f, 0.f);
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, mCurTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return CAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));
}

void CAnimSourceReader::VSetPhase(float phase) {
  mCurTime = CCharAnimTime(
      phase * mSource->GetSteadyStateAnimInfo(mCurTime).GetDuration().GetSeconds());
  if (mSource->HasPOIData()) {
    UpdatePOIStates();
    if (!mCurTime.GreaterThanZero()) {
      mPassedBoolCount = 0;
      mPassedIntCount = 0;
      mPassedParticleCount = 0;
      mPassedSoundCount = 0;
    }
  }
}

bool CAnimSourceReader::VSupportsReverseView() const { return true; }

CAdvancementResults
CAnimSourceReader::VGetAdvancementResults(const CCharAnimTime& dt,
                                          const CCharAnimTime& startOffset) const {
  const CCharAnimTime previousTime = mCurTime + startOffset;
  CCharAnimTime currentTime = mCurTime + startOffset;
  const CCharAnimTime& duration = mSource->GetAnimationDuration();
  if (previousTime >= duration) {
    return CAdvancementResults(dt, CAdvancementDeltas());
  }
  if (dt.EqualsZero()) {
    return CAdvancementResults(CCharAnimTime::ZeroFlat(), CAdvancementDeltas());
  }

  currentTime += dt;
  CCharAnimTime remainingTime = CCharAnimTime::ZeroFlat();
  if (currentTime > duration) {
    remainingTime = currentTime - duration;
    currentTime = duration;
  }

  const CSegId root = CSegId::Root();
  CVector3f offset(0.f, 0.f, 0.f);
  const CQuaternion oldRotation = mSource->GetRotation(root, previousTime);
  const CQuaternion newRotation = mSource->GetRotation(root, currentTime);
  const CQuaternion inverseOldRotation = oldRotation.BuildInverted();
  if (mSource->HasOffset(root)) {
    const CVector3f oldOffset = mSource->GetOffset(root, previousTime);
    const CVector3f newOffset = mSource->GetOffset(root, currentTime);
    offset = newOffset - oldOffset;
    const CQuaternion inverseNewRotation = newRotation.BuildInverted();
    const CMatrix3f inverseRotation = inverseNewRotation.BuildTransform();
    offset = inverseRotation * offset;
  }
  return CAdvancementResults(remainingTime,
                             CAdvancementDeltas(offset, newRotation * inverseOldRotation));
}
