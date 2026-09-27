#ifndef _CPIRATERAGDOLL
#define _CPIRATERAGDOLL

#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CSpacePirate;

class CPirateRagDoll : public CRagDoll {
public:
  void Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& mData) override;
  void Update(CStateManager& mgr, float dt, float waterTop) override;
  void PreRender(const CVector3f& pos, CModelData& mData) override;
  CPirateRagDoll(CStateManager& mgr, CSpacePirate* pirate, ushort thudSfx, uint flags);
  ~CPirateRagDoll() {}

  CVector3f& TorsoImpulse() { return mTorsoImpulse; }

private:
  CSpacePirate* mSpacePirate;
  ushort mThudSfx;
  float mSfxTimer;
  CVector3f mLastSFXPos;
  CVector3f mTorsoImpulse;
  rstl::reserved_vector< TUniqueId, 4 > mWaypoints;
  rstl::reserved_vector< int, 4 > mWpParticleIdxs;
  bool mInitSfx : 1;
};
CHECK_SIZEOF(CPirateRagDoll, 0xb4)

#endif // _CPIRATERAGDOLL
