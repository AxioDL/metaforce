#ifndef __URDE_MP1_CACTORCONTRAPTION_HPP__
#define __URDE_MP1_CACTORCONTRAPTION_HPP__

#include "World/CScriptActor.hpp"
#include "World/CDamageInfo.hpp"

namespace urde
{
namespace MP1
{
class CActorContraption : public CScriptActor
{
    /* AKA Why Zoid?!?!?!? */

    TToken<CGenDescription> x300_;
    CDamageInfo x30c_;
public:
    CActorContraption(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                      const zeus::CAABox&, const CMaterialList&, float, float, const CHealthInfo&,
                      const CDamageVulnerability&, const CActorParameters&, ResId, const CDamageInfo&, bool);

    void Accept(IVisitor &visitor);
};
}
}

#endif // __URDE_MP1_CACTORCONTRAPTION_HPP__
