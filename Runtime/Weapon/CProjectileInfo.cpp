#include "CProjectileInfo.hpp"
#include "World/CDamageInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde {

CProjectileInfo::CProjectileInfo(urde::CInputStream& in)
: x0_weaponDescription(g_SimplePool->GetObj({SBIG('WPSC'), CAssetId(in)})), xc_damageInfo(in) {}

CProjectileInfo::CProjectileInfo(CAssetId proj, const CDamageInfo& dInfo)
: x0_weaponDescription(g_SimplePool->GetObj({SBIG('WPSC'), proj})), xc_damageInfo(dInfo) {}

zeus::CVector3f CProjectileInfo::PredictInterceptPos(const zeus::CVector3f&, const zeus::CVector3f&, const CPlayer&,
                                                     bool) {
  return {};
}

} // namespace urde
