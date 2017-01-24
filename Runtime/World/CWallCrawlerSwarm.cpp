#include "World/CWallCrawlerSwarm.hpp"
#include "World/CActorParameters.hpp"
#include "Collision/CMaterialList.hpp"
#include "TCastTo.hpp"

namespace urde
{

CMaterialList MakeMaterialList()
{
    return CMaterialList(EMaterialTypes::Scannable, EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable,
                         EMaterialTypes::ExcludeFromLineOfSightTest);
}

CWallCrawlerSwarm::CWallCrawlerSwarm(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                                     const zeus::CVector3f&, const zeus::CTransform& xf, u32, const CAnimRes&, u32, u32,
                                     u32, u32, u32, u32, const CDamageInfo&, const CDamageInfo&, float, float, float,
                                     float, u32, u32, float, float, float, float, float, float, float, float, float,
                                     u32, float, float, float, const CHealthInfo&, const CDamageVulnerability&, u32,
                                     u32, const CActorParameters& aParams)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), MakeMaterialList(), aParams, kInvalidUniqueId)
{
}

void CWallCrawlerSwarm::Accept(IVisitor& visitor) { visitor.Visit(this); }
}
