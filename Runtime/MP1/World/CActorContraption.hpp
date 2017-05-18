#ifndef __URDE_MP1_CACTORCONTRAPTION_HPP__
#define __URDE_MP1_CACTORCONTRAPTION_HPP__

#include "World/CScriptActor.hpp"
#include "World/CDamageInfo.hpp"

namespace urde
{
class CFlameThrower;
namespace MP1
{
class CActorContraption : public CScriptActor
{
    /* AKA Why Zoid?!?!?!? */
    std::vector<std::pair<TUniqueId, std::string>> x2e4_children;
    TToken<CGenDescription> x300_flameThrowerGen;
    ResId x308_partId;
    CDamageInfo x30c_dInfo;
public:
    CActorContraption(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                      const zeus::CAABox&, const CMaterialList&, float, float, const CHealthInfo&,
                      const CDamageVulnerability&, const CActorParameters&, ResId, const CDamageInfo&, bool);

    void Accept(IVisitor &visitor);

    void Think(float, CStateManager &);
    void DoUserAnimEvent(CStateManager &, CInt32POINode &, EUserEventType);
    CFlameThrower* CreateFlameThrower(const std::string&, CStateManager&);
};
}
}

#endif // __URDE_MP1_CACTORCONTRAPTION_HPP__
