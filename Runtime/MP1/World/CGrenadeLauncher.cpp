#include "Runtime/MP1/World/CGrenadeLauncher.hpp"

#include "Runtime/Character/CPASAnimParm.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPlayer.hpp"

namespace urde {
namespace MP1 {
CGrenadeLauncher::CGrenadeLauncher(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& bounds,
                                   const CHealthInfo& healthInfo, const CDamageVulnerability& vulnerability,
                                   const CActorParameters& actParams, TUniqueId otherId,
                                   const CGrenadeLauncherData& data, float f1)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData), {EMaterialTypes::Character, EMaterialTypes::Solid}, bounds,
                {1000.f}, actParams, 0.3f, 0.1f)
, x25c_(healthInfo)
, x264_vulnerability(vulnerability)
, x2cc_otherId(otherId)
, x2d0_data(data)
, x328_cSphere({{}, mData.GetScale().z()}, {EMaterialTypes::Character, EMaterialTypes::Solid})
, x350_actParms(actParams)
, x3c0_particleGenDesc(g_SimplePool->GetObj({SBIG('PART'), data.x40_}))
, x3d8_(actParams.GetThermalMag())
, x3f8_(f1) {
  GetModelData()->EnableLooping(true);
  const CPASDatabase& pasDatabase = GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (int i = 0; i < 4; i++) {
    const auto result = pasDatabase.FindBestAnimation({22, CPASAnimParm::FromEnum(i)}, -1);
    x3c8_animIds[i] = result.second;
  }
}

zeus::CVector3f CGrenadeLauncher::GrenadeTarget(const CStateManager& mgr) {
  const zeus::CVector3f& aim = mgr.GetPlayer().GetAimPosition(mgr, 1.f);
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    return aim - zeus::CVector3f{0.f, 0.f, 0.5f * mgr.GetPlayer().GetEyeHeight()};
  }
  return aim;
}

void CGrenadeLauncher::CalculateGrenadeTrajectory(const zeus::CVector3f& target, const zeus::CVector3f& origin,
                                                  const SGrenadeTrajectoryInfo& info, float& angleOut,
                                                  float& velocityOut) {
  float angle = info.x8_angleMin;
  float velocity = info.x0_;
  float delta = std::max(0.01f, 0.1f * (info.xc_angleMax - info.x8_angleMin));
  zeus::CVector3f dist = target - origin;
  float distXYMag = dist.toVec2f().magnitude();
  float qwSq = info.x0_ * info.x0_;
  float qxSq = info.x4_ * info.x4_;
  float gravAdj = distXYMag * ((0.5f * CPhysicsActor::GravityConstant()) * distXYMag);
  float currAngle = info.x8_angleMin;
  float leastResult = FLT_MAX;
  while (info.xc_angleMax >= currAngle) {
    float cos = std::cos(currAngle);
    float sin = std::sin(currAngle);
    float result = (distXYMag * (cos * sin) - (dist.z() * (cos * cos)));
    if (result > FLT_EPSILON) {
      float div = gravAdj / result;
      if (qwSq <= result && result <= qxSq) {
        angle = currAngle;
        velocity = std::sqrt(div);
        break;
      }
      if (result <= qxSq) {
        result = qwSq - result;
      } else {
        result = result - qxSq;
      }
      if (result < leastResult) {
        angle = currAngle;
        velocity = std::sqrt(div);
        leastResult = result;
      }
    }
    currAngle += delta;
  }
  angleOut = angle;
  velocityOut = velocity;
}
} // namespace MP1
} // namespace urde
