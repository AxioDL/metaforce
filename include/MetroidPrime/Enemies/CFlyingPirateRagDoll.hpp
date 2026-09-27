#ifndef _CFLYINGPIRATERAGDOLL
#define _CFLYINGPIRATERAGDOLL

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "MetroidPrime/CRagDoll.hpp"

class CFlyingPirate;

class CFlyingPirateRagDoll : public CRagDoll {
public:
  ~CFlyingPirateRagDoll();
  void Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData) override;
  void Update(CStateManager& mgr, float dt, float waterTop) override;
  void PreRender(const CVector3f& pos, CModelData& mData) override;

  CFlyingPirateRagDoll(CStateManager& mgr, CFlyingPirate* actor, ushort impactSfx, ushort spinSfx);

private:
  CFlyingPirate* mActor;
  float x70_;
  CVector3f x74_;
  float x80_;
  float x84_;
  ushort mImpactSfx;
  float mImpactTimer;
  CVector3f mLastImpactPos;
  ushort mSpinSfx;
  CSfxHandle mSpinEmitter;
  CVector3f xa4_;
  bool mFirstImpact : 1;
};
CHECK_SIZEOF(CFlyingPirateRagDoll, 0xB4)

#endif // _CFLYINGPIRATERAGDOLL
