#include "MetroidPrime/CParticleGenInfo.hpp"

#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"

#include "Kyoto/Particles/CParticleGen.hpp"

CParticleGenInfo::CParticleGenInfo(const SObjectTag& part, int frameCount,
                                   const rstl::string& boneName, const CVector3f& scale,
                                   CParticleData::EParentedMode parentMode, int flags,
                                   EParticleGenType type)
: mPart(part)
, mSeconds(frameCount * (1.0f / 60.f))
, mBoneName(boneName)
, mCurTime(0.f)
, mActive(false)
, mParentMode(parentMode)
, mFlags(flags)
, mParticleScale(scale)
, mFinishTime(0.f)
, mGrabInitialData(false)
, mTransform(CTransform4f::Identity())
, mOffset(0.f, 0.f, 0.f)
, mType(type) {}

TUniqueId _initializeLight(const rstl::ncrc_ptr< CParticleGen >& system, CStateManager& stateMgr,
                           TAreaId areaId, int lightId) {
  if (system->SystemHasLight()) {
    TUniqueId ret = stateMgr.AllocateUniqueId();
    stateMgr.AddObject(rs_new CGameLight(
        ret, areaId, false, rstl::string_l("ParticleLight"),
        CTransform4f(system->GetOrientation().BuildMatrix3f(), system->GetTranslation()),
        kInvalidUniqueId, system->GetLight(), lightId, 0, 0.f));

    return ret;
  } else {
    return kInvalidUniqueId;
  }
}
