#ifndef __URDE_CPOWERBOMB_HPP__
#define __URDE_CPOWERBOMB_HPP__

#include "CWeapon.hpp"
#include "CToken.hpp"

namespace urde
{

class CElementGen;
class CPowerBomb : public CWeapon
{
    static const zeus::CColor kFadeColor;
    bool x158_24_canStartFilter : 1;
    bool x158_25_filterEnabled : 1;
    float x15c_curTime = 0.f;
    float x160_curRadius = 0.f;
    float x164_radiusIncrement;
    std::unique_ptr<CElementGen> x168_particle;
    float x16c_radius;
public:
    CPowerBomb(const TToken<CGenDescription>& particle, TUniqueId uid, TAreaId aid,
               TUniqueId playerId, const zeus::CTransform& xf, const CDamageInfo& dInfo);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager&);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    void Render(const CStateManager&) const {}
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const { return {}; }
    void Touch(CActor&, CStateManager&) { /*x158_24_canStartFilter; */}
    float GetCurTime() const { return x15c_curTime; }
    void ApplyDynamicDamage(const zeus::CVector3f&, CStateManager&);
};

}

#endif // __URDE_CPOWERBOMB_HPP__
