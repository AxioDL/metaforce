#ifndef _CCAMERASHAKEDATA
#define _CCAMERASHAKEDATA

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"

struct SCameraShakePoint {
  uint mUseEnvelope;
  float mValue;
  float mMagnitude;
  float mAttackTime;
  float mSustainTime;
  float mDuration;

  SCameraShakePoint(int flags, float attackTime, float sustainTime, float duration,
                    float magnitude);

  static SCameraShakePoint NoMotion();
  void Update(float curTime);
  float GetValue() const;
};
CHECK_SIZEOF(SCameraShakePoint, 0x18)

class CCameraShakerComponent {
public:
  virtual ~CCameraShakerComponent() {}

  CCameraShakerComponent(int flags, const SCameraShakePoint& am, const SCameraShakePoint& fm);

  static CCameraShakerComponent NoMotion();
  void UpdateMotion(float curTime, float duration, float distAtt);
  float GetValue() const { return mValue; }
  bool IsModulated() const { return (mUseModulation & 1) != 0; }
  float GetAmplitude() const { return mAm.GetValue(); }
  float GetSeverity() const { return mFm.GetValue(); }

private:
  uint mUseModulation;
  SCameraShakePoint mAm;
  SCameraShakePoint mFm;
  float mValue;
};
CHECK_SIZEOF(CCameraShakerComponent, 0x3c)

class CStateManager;
class CCameraShakeData {
public:
  static CCameraShakeData EatOldCameraShakerData(CInputStream& in);
  CCameraShakeData(float duration, float sfxDist, int flags, const CVector3f& sfxPos,
                   const CCameraShakerComponent& shakerX, const CCameraShakerComponent& shakerY,
                   const CCameraShakerComponent& shakerZ);

  void SetId(int id) { mShakerId = id; }
  int GetId() const { return mShakerId; }
  void Update(float dt, CStateManager& mgr);
  float GetMaxAmplitude() const;
  float GetMaxSeverity() const;
  void SetSfxPositionAndDistance(float distance, CVector3f pos);

  static CCameraShakeData SoftHorizShake(float duration, float magnitude);
  static CCameraShakeData SoftBothAxesShake(float duration, float magnitude);
  static CCameraShakeData HardHorizShake(float duration, float magnitude);
  static CCameraShakeData HardVertShake(float duration, float magnitude);
  static CCameraShakeData HardBothAxesShake(float duration, float magnitude);
  static CCameraShakeData HardHorizShakeDistance(CVector3f pos, float duration, float magnitude,
                                                 float distance);
  static CCameraShakeData HardVertShakeDistance(float duration, float magnitude, float distance,
                                                CVector3f pos);

  float GetDuration() const { return mDuration; }
  float GetCurTime() const { return mCurTime; }
  CVector3f GetPoint() const;
  bool Done() const { return mCurTime >= mDuration; }
  uint GetFlags() const { return mFlags; }
  const CVector3f& GetSfxPos() const { return mSfxPos; }

  static CCameraShakeData skSoftRecoil;
  static CCameraShakeData skHardRecoil;

private:
  float mDuration;
  float mCurTime;
  CCameraShakerComponent mShakerX;
  CCameraShakerComponent mShakerY;
  CCameraShakerComponent mShakerZ;
  int mShakerId;
  uint mFlags;
  CVector3f mSfxPos;
  float mSfxDist;
};
CHECK_SIZEOF(CCameraShakeData, 0xd4)

class CInputStream;
CCameraShakeData LoadCameraShakeData(CInputStream& in);
CCameraShakerComponent LoadNewCameraShakerComponent(CInputStream& in);

#endif // _CCAMERASHAKEDATA
