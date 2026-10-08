#ifndef _CWIIMOTIONPROCESSOR
#define _CWIIMOTIONPROCESSOR

#include "types.h"

#include "Kyoto/Input/CInputFilter.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

#include <revolution/wpad.h>

CHECK_SIZEOF(WPADFSStatus, 0x32)

// Motion class and method names are inferred from the Trilogy implementation.
class CBiquadFilter {
public:
  CBiquadFilter()
  : mPreviousInput(0.f)
  , mOlderInput(0.f)
  , mPreviousOutput(0.f)
  , mOlderOutput(0.f)
  , mInitialized(false) {}

  float Filter(float value, float b0, float b1, float b2, float a1, float a2);

private:
  float ProcessSample(float value, float b0, float b1, float b2, float a1, float a2);

  float mPreviousInput;
  float mOlderInput;
  float mPreviousOutput;
  float mOlderOutput;
  bool mInitialized;
};
CHECK_SIZEOF(CBiquadFilter, 0x14)

class CVectorBiquadFilter {
public:
  CVectorBiquadFilter() : mFilteredVector(CVector3f::Zero()), mFilteredMagnitude(0.f) {}

  void Update(const CVector3f& value, float magnitude);
  void SetCoefficients(float b0, float b1, float b2, float a1, float a2);
  const CVector3f& GetFilteredVector() const { return mFilteredVector; }
  float GetFilteredMagnitude() const { return mFilteredMagnitude; }

private:
  CBiquadFilter mXFilter;
  CBiquadFilter mYFilter;
  CBiquadFilter mZFilter;
  CBiquadFilter mMagnitudeFilter;
  float mFeedforward[3];
  float mFeedback[2];
  CVector3f mFilteredVector;
  float mFilteredMagnitude;
};
CHECK_SIZEOF(CVectorBiquadFilter, 0x74)

class CMotionSampleHistory {
public:
  CMotionSampleHistory();
  void AddSample(const CVector3f& acceleration, const CVector3f& gravityUnits, float deadzone);
  int GetSampleCount() const { return mSamples.size(); }
  const CVector3f& GetSample(uint index) const { return mSamples[index]; }
  uint GetWriteIndex() const { return mWriteIndex; }

private:
  uint mWriteIndex;
  rstl::reserved_vector< CVector3f, 30 > mSamples;
};
CHECK_SIZEOF(CMotionSampleHistory, 0x170)

class CMotionGesture {
public:
  CMotionGesture(int axis, int negativeDirection, float startThreshold, float releaseThreshold)
  : mStartThreshold(startThreshold)
  , mReleaseThreshold(releaseThreshold)
  , mCompletedImpulse(0.f)
  , mCurrentImpulse(0.f)
  , mAxis(axis)
  , mNegativeDirection(negativeDirection)
  , mActive(false)
  , mCompleted(false) {}

  void Reset();
  void Update(const CVector3f& acceleration, float dt);
  bool IsCompleted() const { return mCompleted; }
  float GetCompletedImpulse() const { return mCompletedImpulse; }

private:
  float mStartThreshold;
  float mReleaseThreshold;
  float mCompletedImpulse;
  float mCurrentImpulse;
  int mAxis;
  int mNegativeDirection;
  bool mActive : 1;
  bool mCompleted : 1;
};
CHECK_SIZEOF(CMotionGesture, 0x1c)

class CMotionDeviceTracker {
public:
  CMotionDeviceTracker(int device, int channel, int orientationMode, int mode, int fullAngleMode,
                       float motionThreshold);
  void Reset();
  void Update(const WPADFSStatus& status, const CVector3f& gravityUnits, float dt);
  float Filter(int filter, float value);
  CMotionGesture& PositiveZ() { return mPositiveZ; }
  CMotionGesture& NegativeZ() { return mNegativeZ; }
  CMotionGesture& NegativeX() { return mNegativeX; }
  CMotionGesture& PositiveX() { return mPositiveX; }
  uint GetSwingMask() const;
  float GetMotionIntegral() const { return mMotionIntegral; }
  float GetWrappedRoll() const { return mWrappedRoll; }
  float GetWrappedPitch() const { return mWrappedPitch; }
  float GetContinuousRoll() const { return mContinuousRoll; }
  float GetContinuousPitch() const { return mContinuousPitch; }

private:
  int mDevice;
  int mChannel;
  float mMotionMagnitude;
  CVector3f mNormalizedAcceleration;
  CVector3f mFilteredAcceleration;
  float mMotionThreshold;
  float mMotionIntegral;
  CVector3f mPositiveAxisIntegrals;
  float mRoll;
  float mWrappedRoll;
  float mContinuousRoll;
  float mPreviousRoll;
  float mRollWrapOffset;
  float mPitch;
  float mWrappedPitch;
  float mContinuousPitch;
  float mPreviousPitch;
  float mPitchWrapOffset;
  int mOrientationMode;
  int mMode;
  int mFullAngleMode;
  CMotionGesture mPositiveZ;
  CMotionGesture mNegativeZ;
  CMotionGesture mNegativeX;
  CMotionGesture mPositiveX;
  CMotionSampleHistory mHistory;
  rstl::reserved_vector< CAdaptiveInputFilter, 10 > mFilters;
};
CHECK_SIZEOF(CMotionDeviceTracker, 0x980)

struct KPADStatus;

class CWiiMotionProcessor {
public:
  struct SDeviceSample {
    SDeviceSample()
    : mAcceleration(CVector3f::Zero())
    , mLowPassAcceleration(CVector3f::Zero())
    , mLowPassMagnitude(0.f)
    , mHighPassAcceleration(CVector3f::Zero())
    , mHighPassMagnitude(0.f)
    , mBasisZ(CVector3f::Zero())
    , mBasis(CMatrix3f::Identity()) {}

    SDeviceSample& operator+=(const SDeviceSample& other) {
      mAcceleration += other.mAcceleration;
      mLowPassAcceleration += other.mLowPassAcceleration;
      mLowPassMagnitude += other.mLowPassMagnitude;
      mHighPassAcceleration += other.mHighPassAcceleration;
      mHighPassMagnitude += other.mHighPassMagnitude;
      mBasisZ += other.mBasisZ;
      return *this;
    }

    SDeviceSample& operator*=(float scale) {
      mAcceleration *= scale;
      mLowPassAcceleration *= scale;
      mLowPassMagnitude *= scale;
      mHighPassAcceleration *= scale;
      mHighPassMagnitude *= scale;
      mBasisZ *= scale;
      return *this;
    }

    CVector3f mAcceleration;
    CVector3f mLowPassAcceleration;
    float mLowPassMagnitude;
    CVector3f mHighPassAcceleration;
    float mHighPassMagnitude;
    CVector3f mBasisZ;
    CMatrix3f mBasis;
  };

  struct SPairedSample {
    void Update(CWiiMotionProcessor& processor, const WPADFSStatus& status);

    SDeviceSample mWiimote;
    SDeviceSample mNunchuk;

  private:
    void UpdateDeviceSample(CVectorBiquadFilter& lowPass, CVectorBiquadFilter& highPass,
                            const WPADFSStatus& status, SDeviceSample& result, int device);
  };

  struct SMotionPulseState {
    SMotionPulseState() : mImpulseTime(0.f), mShakeTime(0.f), mShakeMask(0) {}

    float mImpulseTime;
    float mShakeTime;
    uint mShakeMask;
  };

  CWiiMotionProcessor(int channel);
  void Update(const WPADFSStatus& status, const KPADStatus& kpadStatus, float dt);
  void UpdateAverage();
  uint UpdateDirectionalGestures(CMotionDeviceTracker& tracker, const WPADFSStatus& status,
                                 int device, float dt);
  uint UpdatePulseGestures(const SDeviceSample& sample, SMotionPulseState& state, int device,
                           float dt);
  void ConfigureFiltersAndCalibration(int channel);
  uint GetSwingMask(int device) const;
  uint GetMotionIntegralMask(const CMotionDeviceTracker& tracker, int device, float dt) const;
  const CMotionDeviceTracker& GetWiimoteTracker() const { return mWiimote; }
  const CMotionDeviceTracker& GetNunchukTracker() const { return mNunchuk; }
  uint GetMotionMask() const { return mMotionMask; }
  uint GetSwingMask() const { return mSwingMask; }
  static CVector3f GetAcceleration(const WPADFSStatus& status, int device);
  static CVector3f ApplyAccelerationDeadzone(const CVector3f& acceleration,
                                             const CVector3f& gravityUnits, float deadzone);
  static CVector3f NormalizeAcceleration(const CVector3f& acceleration,
                                         const CVector3f& gravityUnits);

private:
  friend struct SPairedSample;

  rstl::reserved_vector< SPairedSample, 301 > mHistory;
  int mPreviousIndex;
  int mWriteIndex;
  WPADFSStatus mStatus;
  SPairedSample mLatestSample;
  CVectorBiquadFilter mWiimoteLowPass;
  CVectorBiquadFilter mWiimoteHighPass;
  CVectorBiquadFilter mNunchukLowPass;
  CVectorBiquadFilter mNunchukHighPass;
  CMotionDeviceTracker mWiimote;
  CMotionDeviceTracker mNunchuk;
  SPairedSample mAverageSample;
  float mSampleWeight;
  bool mFlag;
  float xeee0_;
  SMotionPulseState mWiimotePulses;
  SMotionPulseState mNunchukPulses;
  uint mMotionMask;
  uint mSwingMask;
  CVector3f mWiimoteGravityUnits;
  CVector3f mNunchukGravityUnits;
};
CHECK_SIZEOF(CWiiMotionProcessor, 0xef1c)
NESTED_CHECK_SIZEOF(CWiiMotionProcessor, SDeviceSample, 0x5c)
NESTED_CHECK_SIZEOF(CWiiMotionProcessor, SPairedSample, 0xb8)
NESTED_CHECK_SIZEOF(CWiiMotionProcessor, SMotionPulseState, 0xc)

#endif // _CWIIMOTIONPROCESSOR
