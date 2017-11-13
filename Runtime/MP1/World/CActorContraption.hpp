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
    std::vector<std::pair<TUniqueId, std::string>> x2e8_children;
    TToken<CGenDescription> x300_flameThrowerGen;
    CAssetId x308_flameFxId;
    CDamageInfo x30c_dInfo;
public:
    CActorContraption(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                      const zeus::CAABox&, const CMaterialList&, float, float, const CHealthInfo&,
                      const CDamageVulnerability&, const CActorParameters&, CAssetId, const CDamageInfo&, bool);

    void Accept(IVisitor &visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void Think(float, CStateManager &);
    void DoUserAnimEvent(CStateManager &, CInt32POINode &, EUserEventType, float dt);
    CFlameThrower* CreateFlameThrower(std::string_view, CStateManager&);
    void ResetFlameThrowers(CStateManager& mgr);
};
}
}

#endif // __URDE_MP1_CACTORCONTRAPTION_HPP__
