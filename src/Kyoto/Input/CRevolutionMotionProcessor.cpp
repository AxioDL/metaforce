#include "Kyoto/Input/CRevolutionMotionProcessor.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

#include <dolphin/os/OSFastCast.h>
#include <math.h>

CMotionSampleHistory::CMotionSampleHistory() : mWriteIndex(0), mSamples(CVector3f::Zero()) {}

void CMotionSampleHistory::AddSample(const CVector3f& acceleration, const CVector3f& gravityUnits,
                                     float deadzone) {
  mSamples[mWriteIndex] = CRevolutionMotionProcessor::NormalizeAcceleration(
      CRevolutionMotionProcessor::ApplyAccelerationDeadzone(acceleration, gravityUnits, deadzone),
      gravityUnits);
  mWriteIndex = (mWriteIndex + 1) % 30;
}

uint CMotionDeviceTracker::GetSwingMask() const {
  uint maximumIndex;
  uint minimumIndex;
  uint result = 0;
  {
    float maximum = 0.f;
    float minimum = 0.f;
    maximumIndex = 0;
    minimumIndex = 0;
    float upper = 0.3f;
    float lower = -0.2f;
    uint index = mHistory.GetWriteIndex() == 0 ? 29 : mHistory.GetWriteIndex() - 1;
    for (int i = 0; i < mHistory.GetSampleCount(); ++i) {
      const CVector3f& sample = mHistory.GetSample(index);
      if (sample.GetY() > maximum) {
        maximum = sample.GetY();
        maximumIndex = index;
      }
      if (sample.GetY() < minimum) {
        minimum = sample.GetY();
        minimumIndex = index;
      }
      index = index == 0 ? 29 : index - 1;
    }
    if (maximum > upper && minimum < lower && maximumIndex < minimumIndex) {
      result |= 2;
    } else if (maximum > upper && minimum < lower && minimumIndex < maximumIndex) {
      result |= 1;
    }
  }

  {
    float maximum = 0.f;
    float minimum = 0.f;
    maximumIndex = 0;
    minimumIndex = 0;
    float upper = 0.2f;
    float lower = -0.1f;
    uint index = mHistory.GetWriteIndex() == 0 ? 29 : mHistory.GetWriteIndex() - 1;
    for (int i = 0; i < mHistory.GetSampleCount(); ++i) {
      const CVector3f& sample = mHistory.GetSample(index);
      if (sample.GetX() > maximum) {
        maximum = sample.GetX();
        maximumIndex = index;
      }
      if (sample.GetX() < minimum) {
        minimum = sample.GetX();
        minimumIndex = index;
      }
      index = index == 0 ? 29 : index - 1;
    }
    if (maximum > upper && minimum < lower && maximumIndex < minimumIndex) {
      result |= 8;
    } else if (maximum > upper && minimum < lower && minimumIndex < maximumIndex) {
      result |= 4;
    }
  }

  {
    float maximum = 0.f;
    float minimum = 0.f;
    maximumIndex = 0;
    minimumIndex = 0;
    float upper = 0.1f;
    float lower = 0.f;
    uint index = mHistory.GetWriteIndex() == 0 ? 29 : mHistory.GetWriteIndex() - 1;
    for (int i = 0; i < mHistory.GetSampleCount(); ++i) {
      const CVector3f& sample = mHistory.GetSample(index);
      if (sample.GetZ() > maximum) {
        maximum = sample.GetZ();
        maximumIndex = index;
      }
      if (sample.GetZ() < minimum) {
        minimum = sample.GetZ();
        minimumIndex = index;
      }
      index = index == 0 ? 29 : index - 1;
    }
    if (maximum > upper && minimum < lower && maximumIndex < minimumIndex) {
      result |= 16;
    } else if (maximum > upper && minimum < lower && minimumIndex < maximumIndex) {
      result |= 32;
    }
  }
  return result;
}

CMotionDeviceTracker::CMotionDeviceTracker(int device, int channel, int orientationMode, int mode,
                                           int fullAngleMode, float motionThreshold)
: mDevice(device)
, mChannel(channel)
, mMotionMagnitude(0.f)
, mNormalizedAcceleration(CVector3f::Zero())
, mFilteredAcceleration(CVector3f::Zero())
, mMotionThreshold(motionThreshold)
, mMotionIntegral(0.f)
, mPositiveAxisIntegrals(CVector3f::Zero())
, mRoll(0.f)
, mWrappedRoll(0.f)
, mContinuousRoll(0.f)
, mPreviousRoll(0.f)
, mRollWrapOffset(0.f)
, mPitch(0.f)
, mWrappedPitch(0.f)
, mContinuousPitch(0.f)
, mPreviousPitch(0.f)
, mPitchWrapOffset(0.f)
, mOrientationMode(orientationMode)
, mMode(mode)
, mFullAngleMode(fullAngleMode)
, mPositiveZ(2, 0, 1.2f, 0.4f)
, mNegativeZ(2, 1, 1.6f, 0.2f)
, mNegativeX(0, 1, 1.6f, 0.2f)
, mPositiveX(0, 0, 1.6f, 0.2f) {
  for (int i = 0; i < 10; ++i) {
    mFilters.push_back(CAdaptiveInputFilter(1, 0, 0, 0.1f));
  }
}

float CBiquadFilter::ProcessSample(float value, float b0, float b1, float b2, float a1, float a2) {
  const float result = b0 * value + b1 * mPreviousInput + b2 * mOlderInput -
                       a1 * mPreviousOutput - a2 * mOlderOutput;
  mOlderInput = mPreviousInput;
  mPreviousInput = value;
  mOlderOutput = mPreviousOutput;
  mPreviousOutput = result;
  return result;
}

float CBiquadFilter::Filter(float value, float b0, float b1, float b2, float a1, float a2) {
  if (!mInitialized) {
    for (int i = 0; i < 100; ++i) {
      ProcessSample(value, b0, b1, b2, a1, a2);
    }
    mInitialized = true;
  }
  return ProcessSample(value, b0, b1, b2, a1, a2);
}

void CVectorBiquadFilter::SetCoefficients(float b0, float b1, float b2, float a1, float a2) {
  mFeedforward[0] = b0;
  mFeedforward[1] = b1;
  mFeedforward[2] = b2;
  mFeedback[0] = a1;
  mFeedback[1] = a2;
}

void CVectorBiquadFilter::Update(const CVector3f& value, float magnitude) {
  mFilteredVector.SetX(mXFilter.Filter(value.GetX(), mFeedforward[0], mFeedforward[1],
                                            mFeedforward[2], mFeedback[0], mFeedback[1]));
  mFilteredVector.SetY(mYFilter.Filter(value.GetY(), mFeedforward[0], mFeedforward[1],
                                             mFeedforward[2], mFeedback[0], mFeedback[1]));
  mFilteredVector.SetZ(mZFilter.Filter(value.GetZ(), mFeedforward[0], mFeedforward[1],
                                             mFeedforward[2], mFeedback[0], mFeedback[1]));
  mFilteredMagnitude =
      mMagnitudeFilter.Filter(magnitude, mFeedforward[0], mFeedforward[1],
                                 mFeedforward[2], mFeedback[0], mFeedback[1]);
}

void CRevolutionMotionProcessor::SPairedSample::Update(CRevolutionMotionProcessor& processor,
                                                const WPADFSStatus& status) {
  UpdateDeviceSample(processor.mWiimoteLowPass, processor.mWiimoteHighPass, status,
                     mWiimote, 0);
  UpdateDeviceSample(processor.mNunchukLowPass, processor.mNunchukHighPass, status,
                     mNunchuk, 1);
}

void CRevolutionMotionProcessor::SPairedSample::UpdateDeviceSample(CVectorBiquadFilter& lowPass,
                                                            CVectorBiquadFilter& highPass,
                                                            const WPADFSStatus& status,
                                                            SDeviceSample& result, int device) {
  result.mAcceleration = GetAcceleration(status, device);
  const float magnitude = CMath::FastSqrtF(result.mAcceleration.MagSquared());
  lowPass.Update(result.mAcceleration, magnitude);
  result.mLowPassAcceleration = lowPass.GetFilteredVector();
  result.mLowPassMagnitude = lowPass.GetFilteredMagnitude();
  highPass.Update(result.mAcceleration, magnitude);
  result.mHighPassAcceleration = highPass.GetFilteredVector();
  result.mHighPassMagnitude = highPass.GetFilteredMagnitude();
  result.mBasisZ =
      CVector3f(result.mBasis.Get02(), result.mBasis.Get12(), result.mBasis.Get22());
}

CVector3f CRevolutionMotionProcessor::GetAcceleration(const WPADFSStatus& status, int device) {
  if (device == 0) {
    const float x = (1.f / 512.f) * status.accX;
    const float y = (1.f / 512.f) * status.accY;
    const float z = (1.f / 512.f) * status.accZ;
    return CVector3f(x, y, z);
  }
  const float x = (1.f / 512.f) * status.fsAccX;
  const float y = (1.f / 512.f) * status.fsAccY;
  const float z = (1.f / 512.f) * status.fsAccZ;
  return CVector3f(x, y, z);
}

void CRevolutionMotionProcessor::UpdateAverage() {
  int sampleCount = 4;
  float weight = mSampleWeight;
  if (sampleCount < weight) {
    weight = sampleCount;
  }
  mSampleWeight = weight;
  int index = mWriteIndex - int(mSampleWeight) + 1;
  if (index < 0) {
    index += 301;
  }
  mAverageSample = SPairedSample();
  CVector3f wiimoteY = CVector3f::Zero();
  CVector3f wiimoteX = CVector3f::Zero();
  CVector3f nunchukY = CVector3f::Zero();
  CVector3f nunchukX = CVector3f::Zero();
  float count = 0.f;
  do {
    const SPairedSample& sample = mHistory[index];
    mAverageSample.mWiimote += sample.mWiimote;
    wiimoteY += sample.mWiimote.mBasis.GetColumn(kDY);
    wiimoteX += sample.mWiimote.mBasis.GetColumn(kDX);
    mAverageSample.mNunchuk += sample.mNunchuk;
    nunchukY += sample.mNunchuk.mBasis.GetColumn(kDY);
    nunchukX += sample.mNunchuk.mBasis.GetColumn(kDX);
    index = (index + 1) % 301;
    count += 1.f;
  } while (index != mWriteIndex);

  const float scale = 1.f / count;
  mAverageSample.mWiimote *= scale;
  wiimoteY.Normalize();
  CVector3f z = CVector3f::Cross(wiimoteX, wiimoteY);
  z.Normalize();
  wiimoteX = CVector3f::Cross(wiimoteY, z);
  const CMatrix3f wiimoteBasis(wiimoteX, wiimoteY, z);
  mAverageSample.mWiimote.mBasis = wiimoteBasis;
  mAverageSample.mNunchuk *= scale;
  nunchukY.Normalize();
  z = CVector3f::Cross(nunchukX, nunchukY);
  z.Normalize();
  nunchukX = CVector3f::Cross(nunchukY, z);
  const CMatrix3f nunchukBasis(nunchukX, nunchukY, z);
  mAverageSample.mNunchuk.mBasis = nunchukBasis;
}

void CMotionGesture::Update(const CVector3f& acceleration, float dt) {
  float axis = 0.f;
  float impulse = 0.f;
  switch (mAxis) {
  case 0:
    axis = acceleration.GetX();
    impulse = acceleration.GetY();
    break;
  case 2:
    axis = acceleration.GetZ();
    impulse = acceleration.GetY();
    break;
  }

  if (mActive) {
    if (mNegativeDirection == 0) {
      if (axis < mReleaseThreshold) {
        mCompleted = true;
        mActive = false;
        mCompletedImpulse = mCurrentImpulse;
        mCurrentImpulse = 0.f;
      }
    } else if (axis > -mReleaseThreshold) {
      mCompleted = true;
      mActive = false;
      mCompletedImpulse = mCurrentImpulse;
      mCurrentImpulse = 0.f;
    }
  } else {
    mCompleted = false;
    if (impulse > 0.f) {
      mCurrentImpulse += impulse * dt;
    } else {
      mCurrentImpulse = 0.f;
    }

    if (mNegativeDirection == 0) {
      if (axis > mStartThreshold) {
        mActive = true;
      }
    } else if (axis < -mStartThreshold) {
      mActive = true;
    }
  }
}

void CMotionGesture::Reset() {
  mStartThreshold = 0.f;
  mReleaseThreshold = 0.f;
  mCurrentImpulse = 0.f;
  mActive = false;
  mCompleted = false;
}

float CMotionDeviceTracker::Filter(int filter, float value) {
  return mFilters[filter].Filter(value);
}

void CMotionDeviceTracker::Reset() {
  mMotionMagnitude = 0.f;
  mNormalizedAcceleration = CVector3f::Zero();
  mFilteredAcceleration = CVector3f::Zero();
  mPositiveAxisIntegrals = CVector3f::Zero();
  mRoll = 0.f;
  mWrappedRoll = 0.f;
  mContinuousRoll = 0.f;
  mPreviousRoll = 0.f;
  mRollWrapOffset = 0.f;
  mPitch = 0.f;
  mWrappedPitch = 0.f;
  mContinuousPitch = 0.f;
  mPreviousPitch = 0.f;
  mPitchWrapOffset = 0.f;
  mNegativeZ.Reset();
  mPositiveZ.Reset();
  mNegativeX.Reset();
  mPositiveX.Reset();
}

void CMotionDeviceTracker::Update(const WPADFSStatus& status, const CVector3f& gravityUnits,
                                  float dt) {
  float deadzone;
  switch (mDevice) {
  case 0:
    mNormalizedAcceleration[kDX] = status.accX;
    mNormalizedAcceleration[kDY] = status.accY;
    mNormalizedAcceleration[kDZ] = status.accZ;
    deadzone = 90.f;
    break;
  case 1:
    mNormalizedAcceleration[kDX] = status.fsAccX;
    mNormalizedAcceleration[kDY] = status.fsAccY;
    mNormalizedAcceleration[kDZ] = status.fsAccZ;
    deadzone = 50.f;
    break;
  default:
    return;
  }
  const CVector3f rawAcceleration = mNormalizedAcceleration;
  mHistory.AddSample(mNormalizedAcceleration, gravityUnits, deadzone);
  mNormalizedAcceleration =
      CRevolutionMotionProcessor::NormalizeAcceleration(mNormalizedAcceleration, gravityUnits);
  {
    const float& limit = 1.f;
    mNormalizedAcceleration.SetX(
        CMath::FastLimit(mNormalizedAcceleration.GetX(), limit));
    mNormalizedAcceleration.SetY(
        CMath::FastLimit(mNormalizedAcceleration.GetY(), limit));
    mNormalizedAcceleration.SetZ(
        CMath::FastLimit(mNormalizedAcceleration.GetZ(), limit));
  }
  switch (mDevice) {
  case 0:
    mFilteredAcceleration.SetX(Filter(0, mNormalizedAcceleration.GetX()));
    mFilteredAcceleration.SetY(Filter(1, mNormalizedAcceleration.GetY()));
    mFilteredAcceleration.SetZ(Filter(2, mNormalizedAcceleration.GetZ()));
    break;
  case 1:
    mFilteredAcceleration.SetX(Filter(5, mNormalizedAcceleration.GetX()));
    mFilteredAcceleration.SetY(Filter(6, mNormalizedAcceleration.GetY()));
    mFilteredAcceleration.SetZ(Filter(7, mNormalizedAcceleration.GetZ()));
    break;
  default:
    return;
  }
  if (mOrientationMode == 1) {
    const float y = mNormalizedAcceleration.GetY();
    mNormalizedAcceleration.SetY(mNormalizedAcceleration.GetX());
    mNormalizedAcceleration.SetX(-y);
  }
  if (mFullAngleMode == 1) {
    const float z = mNormalizedAcceleration.GetZ();
    const float& x = mNormalizedAcceleration.GetX();
    float magnitude = sqrtf(x * x + z * z);
    if (CMath::IsEpsilon(magnitude, 0.f, 0.00001f)) {
      magnitude = 1.f;
    }
    if (mNormalizedAcceleration.GetX() >= 0.f) {
      const float& ratio = mNormalizedAcceleration.GetX() / magnitude;
      const float& limit = 1.f;
      mRoll = static_cast< float >(asin(CMath::FastLimit(ratio, limit)));
    } else {
      const float& limit = 1.f;
      mRoll = -static_cast< float >(
          asin(CMath::FastLimit(-mNormalizedAcceleration.GetX() / magnitude, limit)));
    }
    if (mDevice == 1) {
      mRoll = Filter(8, mRoll);
    } else {
      mRoll = Filter(3, mRoll);
    }
    mWrappedRoll = mRoll;
    if (z < 0.f) {
      if (mNormalizedAcceleration.GetX() < 0.f) {
        mWrappedRoll = -M_PIF - mRoll;
      } else {
        mWrappedRoll = M_PIF - mRoll;
      }
    }
    if (mPreviousRoll > M_PIF / 2.f && mWrappedRoll < -M_PIF / 2.f) {
      mRollWrapOffset += M_2PIF;
    } else if (mPreviousRoll < -M_PIF / 2.f && mWrappedRoll > M_PIF / 2.f) {
      mRollWrapOffset -= M_2PIF;
    }
    mPreviousRoll = mWrappedRoll;
    mContinuousRoll = mWrappedRoll + mRollWrapOffset;

    const float pitchZ = mNormalizedAcceleration.GetZ();
    magnitude = sqrtf(pitchZ * pitchZ +
                      mNormalizedAcceleration.GetY() * mNormalizedAcceleration.GetY());
    magnitude = CMath::FastClamp(0.f, magnitude, 1.f);
    if (CMath::IsEpsilon(magnitude, 0.f, 0.00001f)) {
      magnitude = 1.f;
    }
    if (mNormalizedAcceleration.GetZ() >= 0.f) {
      mPitch = static_cast< float >(asin(mNormalizedAcceleration.GetY() / magnitude));
    } else {
      mPitch = -static_cast< float >(asin(-mNormalizedAcceleration.GetY() / magnitude));
    }
    if (mDevice == 1) {
      mPitch = Filter(9, mPitch);
    } else {
      mPitch = Filter(4, mPitch);
    }
    mWrappedPitch = mPitch;
    if (pitchZ < 0.f) {
      if (mNormalizedAcceleration.GetY() < 0.f) {
        mWrappedPitch = -M_PIF - mPitch;
      } else {
        mWrappedPitch = M_PIF - mPitch;
      }
    }
    if (mPreviousPitch > M_PIF / 2.f && mWrappedPitch < -M_PIF / 2.f) {
      mPitchWrapOffset += M_2PIF;
    } else if (mPreviousPitch < -M_PIF / 2.f && mWrappedPitch > M_PIF / 2.f) {
      mPitchWrapOffset -= M_2PIF;
    }
    mPreviousPitch = mWrappedPitch;
    mContinuousPitch = mWrappedPitch + mPitchWrapOffset;

    CVector3f motion = rawAcceleration;
    motion = CRevolutionMotionProcessor::ApplyAccelerationDeadzone(motion, gravityUnits, deadzone);
    motion = CRevolutionMotionProcessor::NormalizeAcceleration(motion, gravityUnits);
    if (mOrientationMode == 1) {
      mMotionMagnitude = sqrtf(motion.GetX() * motion.GetX() + motion.GetZ() * motion.GetZ());
    } else {
      mMotionMagnitude = sqrtf(motion.GetY() * motion.GetY() + motion.GetZ() * motion.GetZ());
    }
  } else {
    {
      const float& limit = 1.f;
      mNormalizedAcceleration.SetX(
          CMath::FastLimit(mNormalizedAcceleration.GetX(), limit));
    }
    float roll;
    if (mNormalizedAcceleration.GetX() >= 0.f) {
      roll = static_cast< float >(asin(mNormalizedAcceleration.GetX()));
    } else {
      roll = -static_cast< float >(asin(-mNormalizedAcceleration.GetX()));
    }
    int rollFilter = 0;
    if (mDevice == 1) {
      rollFilter = 5;
    }
    mRoll = Filter(rollFilter, roll);
    {
      const float& limit = 1.f;
      mNormalizedAcceleration.SetY(
          CMath::FastLimit(mNormalizedAcceleration.GetY(), limit));
    }
    float pitch;
    if (mNormalizedAcceleration.GetY() >= 0.f) {
      pitch = static_cast< float >(asin(mNormalizedAcceleration.GetY()));
    } else {
      pitch = -static_cast< float >(asin(-mNormalizedAcceleration.GetY()));
    }
    int pitchFilter = 1;
    if (mDevice == 1) {
      pitchFilter = 6;
    }
    mPitch = Filter(pitchFilter, pitch);
  }

  const float& magnitudeRef = mMotionMagnitude;
  const float motionMagnitude = CMath::AbsF(magnitudeRef);
  mMotionIntegral += mMotionMagnitude * dt;
  if (motionMagnitude < 0.1f) {
    mMotionIntegral = 0.f;
  }
  CVector3f motion = rawAcceleration;
  motion = CRevolutionMotionProcessor::ApplyAccelerationDeadzone(motion, gravityUnits, deadzone);
  if (motion.GetX() > 0.1f * dt) {
    mPositiveAxisIntegrals[kDX] += dt * motion.GetX();
  } else {
    mPositiveAxisIntegrals.SetX(0.f);
  }
  if (motion.GetY() > 0.1f * dt) {
    mPositiveAxisIntegrals[kDY] += dt * motion.GetY();
  } else {
    mPositiveAxisIntegrals.SetY(0.f);
  }
  if (motion.GetZ() > 0.1f * dt) {
    mPositiveAxisIntegrals[kDZ] += dt * motion.GetZ();
  } else {
    mPositiveAxisIntegrals.SetZ(0.f);
  }
}

void CRevolutionMotionProcessor::Update(const WPADFSStatus& status, const KPADStatus& kpadStatus,
                                 float dt) {
  mPreviousIndex = mWriteIndex;
  ++mWriteIndex;
  if (mWriteIndex >= 301) {
    mWriteIndex = 0;
  }
  int remaining = mWriteIndex - mPreviousIndex;
  if (mPreviousIndex > mWriteIndex) {
    remaining = mWriteIndex + 301 - mPreviousIndex;
  }
  int index = mPreviousIndex;
  SPairedSample* sample = &mHistory[index];
  for (; remaining != 0;) {
    ++index;
    --remaining;
    mSampleWeight += 0.1f;
    index %= 301;
    sample = &mHistory[index];
    sample->Update(*this, status);
  }
  mWiimote.Update(status, mWiimoteGravityUnits, dt);
  mNunchuk.Update(status, mNunchukGravityUnits, dt);
  uint motionMask = UpdatePulseGestures(sample->mWiimote, mWiimotePulses, 0, dt);
  motionMask |= UpdatePulseGestures(sample->mNunchuk, mNunchukPulses, 1, dt);
  motionMask |= UpdateDirectionalGestures(mWiimote, status, 0, dt);
  motionMask |= UpdateDirectionalGestures(mNunchuk, status, 1, dt);
  motionMask |= GetMotionIntegralMask(mWiimote, 0, dt);
  motionMask |= GetMotionIntegralMask(mNunchuk, 1, dt);
  mStatus = status;
  mLatestSample = mHistory[mWriteIndex];
  mMotionMask = motionMask;
  mSwingMask = GetSwingMask(0) | GetSwingMask(1);
  UpdateAverage();
}

uint CRevolutionMotionProcessor::UpdateDirectionalGestures(CMotionDeviceTracker& tracker,
                                                    const WPADFSStatus& status, int device,
                                                    float dt) {
  uint result = 0;
  if (device == 1) {
    CVector3f acceleration(status.fsAccX, status.fsAccY, status.fsAccZ);
    acceleration = ApplyAccelerationDeadzone(acceleration, mNunchukGravityUnits, 50.f);
    acceleration = NormalizeAcceleration(acceleration, mNunchukGravityUnits);
    const float& limit = 1.f;
    acceleration.SetX(CMath::FastLimit(acceleration.GetX(), limit));
    acceleration.SetY(CMath::FastLimit(acceleration.GetY(), limit));
    acceleration.SetZ(CMath::FastLimit(acceleration.GetZ(), limit));
    tracker.NegativeZ().Update(acceleration, dt);
    tracker.PositiveZ().Update(acceleration, dt);
    tracker.NegativeX().Update(acceleration, dt);
    tracker.PositiveX().Update(acceleration, dt);
    if (tracker.PositiveZ().IsCompleted() && tracker.PositiveZ().GetCompletedImpulse() < -0.05f) {
      result |= 0x100000;
    }
    if (tracker.NegativeZ().IsCompleted() && tracker.NegativeZ().GetCompletedImpulse() > 0.05f) {
      result |= 0x80000;
    }
    if (tracker.NegativeX().IsCompleted() && tracker.NegativeX().GetCompletedImpulse() > 0.05f) {
      result |= 0x200000;
    }
    if (tracker.PositiveX().IsCompleted() && tracker.PositiveX().GetCompletedImpulse() > 0.05f) {
      result |= 0x400000;
    }
  } else {
    CVector3f acceleration(status.accX, status.accY, status.accZ);
    acceleration = ApplyAccelerationDeadzone(acceleration, mWiimoteGravityUnits, 90.f);
    const float& limit = 1.f;
    acceleration.SetX(CMath::FastLimit(acceleration.GetX(), limit));
    acceleration.SetY(CMath::FastLimit(acceleration.GetY(), limit));
    acceleration.SetZ(CMath::FastLimit(acceleration.GetZ(), limit));
    tracker.NegativeZ().Update(acceleration, dt);
    tracker.PositiveZ().Update(acceleration, dt);
    tracker.NegativeX().Update(acceleration, dt);
    tracker.PositiveX().Update(acceleration, dt);
    if (tracker.PositiveZ().IsCompleted() && tracker.PositiveZ().GetCompletedImpulse() > 0.05f) {
      result |= 0x10;
    }
    if (tracker.NegativeZ().IsCompleted() && tracker.NegativeZ().GetCompletedImpulse() > 0.05f) {
      result |= 8;
    }
    if (tracker.NegativeX().IsCompleted()) {
      result |= 0x20;
    }
    if (tracker.PositiveX().IsCompleted()) {
      result |= 0x40;
    }
  }
  return result;
}

uint CRevolutionMotionProcessor::GetMotionIntegralMask(const CMotionDeviceTracker& tracker, int device,
                                                float dt) const {
  uint result = 0;
  if (tracker.GetMotionIntegral() > 0.01f) {
    if (device == 0) {
      result |= 0x80;
    } else {
      result |= 0x800000;
    }
  }
  return result;
}

uint CRevolutionMotionProcessor::UpdatePulseGestures(const SDeviceSample& sample, SMotionPulseState& state,
                                              int device, float dt) {
  uint result = 0;
  if (state.mImpulseTime <= 0.f) {
    if (CMath::AbsF(sample.mHighPassAcceleration.GetX()) > 0.02f) {
      state.mImpulseTime = 5.f / 12.f;
      if (device == 0) {
        result |= 1;
      } else {
        result |= 0x10000;
      }
    }
  } else {
    state.mImpulseTime -= dt;
    if (state.mImpulseTime > 0.f) {
      if (device == 0) {
        result |= 1;
      } else {
        result |= 0x10000;
      }
    } else {
      if (device == 0) {
        result &= ~1;
      } else {
        result &= ~0x10000;
      }
    }
  }

  if (state.mShakeTime <= 0.f) {
    float x;
    float z;
    float threshold = 0.25f;
    if (device == 1) {
      threshold = 0.45f;
    }
    const int count = mHistory.size();
    if (count >= 15) {
      int index = mWriteIndex - 15;
      if (index < 0) {
        index += count;
      }
      float minX = 1000.f;
      float maxX = -1000.f;
      float minZ = 1000.f;
      float maxZ = -1000.f;
      for (; index != mWriteIndex; index = (index + 1) % count) {
        if (device == 0) {
          x = mHistory[index].mWiimote.mLowPassAcceleration.GetX();
          z = mHistory[index].mWiimote.mLowPassAcceleration.GetZ();
        } else {
          x = mHistory[index].mNunchuk.mLowPassAcceleration.GetX();
          z = mHistory[index].mNunchuk.mLowPassAcceleration.GetZ();
        }
        if (x < minX) {
          minX = x;
        }
        if (x > maxX) {
          maxX = x;
        }
        if (z < minZ) {
          minZ = z;
        }
        if (z > maxZ) {
          maxZ = z;
        }
      }
      if (maxX - minX > threshold) {
        state.mShakeTime = 0.25f;
        if (device == 0) {
          state.mShakeMask = 2;
        } else {
          state.mShakeMask = 0x20000;
        }
        result |= state.mShakeMask;
      }
      if (maxZ - minZ > threshold) {
        state.mShakeTime = 0.25f;
        if (device == 0) {
          state.mShakeMask = 4;
        } else {
          state.mShakeMask = 0x40000;
        }
        result |= state.mShakeMask;
      }
    }
  } else {
    state.mShakeTime -= dt;
    if (state.mShakeTime > 0.f) {
      result |= state.mShakeMask;
    } else {
      result &= ~state.mShakeMask;
    }
  }
  return result;
}

uint CRevolutionMotionProcessor::GetSwingMask(int device) const {
  uint result = 0;
  switch (device) {
  case 0:
    result = mWiimote.GetSwingMask();
    break;
  case 1:
    result = mNunchuk.GetSwingMask() << 16;
    break;
  }
  return result;
}

CRevolutionMotionProcessor::CRevolutionMotionProcessor(int channel)
: mHistory(SPairedSample())
, mPreviousIndex(0)
, mWriteIndex(0)
, mWiimote(0, channel, 0, 0, 1, 0.5f)
, mNunchuk(1, channel, 0, 0, 1, 0.5f)
, mSampleWeight(0.f)
, mFlag(true)
, mMotionMask(0)
, mSwingMask(0)
, mWiimoteGravityUnits(CVector3f::One())
, mNunchukGravityUnits(CVector3f::One()) {
  ConfigureFiltersAndCalibration(channel);
}

void CRevolutionMotionProcessor::ConfigureFiltersAndCalibration(int channel) {
  const float sqrt2 = CMath::SqrtF(2.f);
  const float lowCutoff = 1.f / static_cast< float >(tan(0.05f * M_PIF));
  float lowB1;
  const float lowB0 = 1.f / (1.f + sqrt2 * lowCutoff + lowCutoff * lowCutoff);
  lowB1 = 2.f * lowB0;
  const float lowA1 = lowB0 * (2.f * (1.f - lowCutoff * lowCutoff));
  const float lowA2 = lowB0 * (1.f - sqrt2 * lowCutoff + lowCutoff * lowCutoff);
  mWiimoteLowPass.SetCoefficients(lowB0, lowB1, lowB0, lowA1, lowA2);
  mNunchukLowPass.SetCoefficients(lowB0, lowB1, lowB0, lowA1, lowA2);

  const float highCutoff = static_cast< float >(tan(0.25f * M_PIF));
  float highB1;
  const float highB0 = 1.f / (1.f + sqrt2 * highCutoff + highCutoff * highCutoff);
  highB1 = -2.f * highB0;
  const float highA1 = highB0 * (-2.f * (1.f - highCutoff * highCutoff));
  const float highA2 = highB0 * (1.f - sqrt2 * highCutoff + highCutoff * highCutoff);
  mWiimoteHighPass.SetCoefficients(highB0, highB1, highB0, highA1, highA2);
  mNunchukHighPass.SetCoefficients(highB0, highB1, highB0, highA1, highA2);

  WPADAcc gravity;
  WPADGetAccGravityUnit(channel, 0, &gravity);
  const float wx = __OSs16tof32(&gravity.x);
  const float wy = __OSs16tof32(&gravity.y);
  const float wz = __OSs16tof32(&gravity.z);
  mWiimoteGravityUnits = CVector3f(wx, wy, wz);
  WPADGetAccGravityUnit(channel, 1, &gravity);
  const float nx = __OSs16tof32(&gravity.x);
  const float ny = __OSs16tof32(&gravity.y);
  const float nz = __OSs16tof32(&gravity.z);
  mNunchukGravityUnits = CVector3f(nx, ny, nz);
  mWiimote.Reset();
  mNunchuk.Reset();
}
