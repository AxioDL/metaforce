#include "Runtime/World/CSnakeWeedSwarm.hpp"

#include "Runtime/World/CAnimationParameters.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CSnakeWeedSwarm::CSnakeWeedSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                                 const zeus::CVector3f& pos, const zeus::CVector3f& scale,
                                 const CAnimationParameters& animParms, const CActorParameters& actParms, float f1,
                                 float f2, float f3, float f4, float f5, float f6, float f7, float f8, float f9,
                                 float f10, float f11, float f12, float f13, float f14, const CDamageInfo&,
                                 float /*f15*/, u32 w1, u32 w2, u32 w3, u32 w4, u32 w5, u32 w6, float f16)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable), actParms, kInvalidUniqueId) {}

void CSnakeWeedSwarm::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
