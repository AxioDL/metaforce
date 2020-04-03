#include "Runtime/MP1/World/CPhazonPool.hpp"

namespace urde::MP1 {
CPhazonPool::CPhazonPool(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         const zeus::CVector3f& scale, bool active, CAssetId w1, CAssetId w2, CAssetId w3, CAssetId w4,
                         u32 p11, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                         ETriggerFlags triggerFlags, bool p15, float p16, float p17, float p18, float p19)
: CScriptTrigger(uid, name, info, xf.origin, zeus::skNullBox, dInfo, orientedForce, triggerFlags, active, false,
                 false) {}
} // namespace urde::MP1
