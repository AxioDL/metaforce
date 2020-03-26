#include "Runtime/MP1/World/CGrenadeLauncher.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CPASAnimParm.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

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
} // namespace MP1
} // namespace urde
