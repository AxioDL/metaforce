#ifndef _CSTATEMANAGERCONTAINER
#define _CSTATEMANAGERCONTAINER

#include "types.h"

#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CSortedLists.hpp"
#include "MetroidPrime/CWeaponMgr.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/reserved_vector.hpp"

class CStateManagerContainer;

class CStateManagerContainer : public TOneStatic<CStateManagerContainer> {
private:
  friend class CStateManager;
  
  CCameraManager mCameraManager;
  SL::CSortedListManager mSortedListManager;
  CWeaponMgr mWeaponManager;
  CFluidPlaneManager mFluidPlaneManager;
  CEnvFxManager mEnvFxManager;
  CActorModelParticles mActorModelParticles;
  CRumbleManager mRumbleManager;
  rstl::reserved_vector< TUniqueId, 20 > xf344_;
  rstl::reserved_vector< TUniqueId, 20 > xf370_;
  rstl::reserved_vector< TUniqueId, 20 > mRenderLast;
};
CHECK_SIZEOF(CStateManagerContainer, 0xf3c8);

#endif // _CSTATEMANAGERCONTAINER
