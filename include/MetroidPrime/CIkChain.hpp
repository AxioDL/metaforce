#ifndef _CIKCHAIN
#define _CIKCHAIN

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CAnimData;
class CTransform4f;

class CIkChain {
public:
  CIkChain()
  : mBone(CSegId::Invalid())
  , mP1(CSegId::Invalid())
  , mP2(CSegId::Invalid())
  , mP2p1Dir(CVector3f::Forward())
  , mP1BoneDir(CVector3f::Forward())
  , mP2p1Length(1.f)
  , mP1BoneLength(1.f)
  , mHoldRot(CQuaternion::NoRotation())
  , mHoldPos(CVector3f::Zero())
  , mTime(0.f)
  , mActivated(false) {}

  bool GetActive() const { return mActivated; }
  void Update(float dt);
  void Deactivate();
  void Activate(const CAnimData& data, CSegId bone, const CTransform4f& xf);
  void PreRender(CAnimData& data, const CTransform4f& xf, const CVector3f& scale);
  void Solve(CQuaternion& q1, CQuaternion& q2, const CVector3f& pos);

private:
  CSegId mBone;
  CSegId mP1;
  CSegId mP2;
  CVector3f mP2p1Dir;
  CVector3f mP1BoneDir;
  float mP2p1Length;
  float mP1BoneLength;
  CQuaternion mHoldRot;
  CVector3f mHoldPos;
  float mTime;
  bool mActivated : 1;
};
CHECK_SIZEOF(CIkChain, 0x48)

#endif // _CIKCHAIN
