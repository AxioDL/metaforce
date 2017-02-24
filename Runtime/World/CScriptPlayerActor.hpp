#ifndef __URDE_CSCRIPTPLAYERACTOR_HPP__
#define __URDE_CSCRIPTPLAYERACTOR_HPP__

#include "CScriptActor.hpp"

namespace urde
{
class CScriptPlayerActor : public CScriptActor
{
    CAnimRes x2e8_;
    u32 x304_;
    s32 x308_ = -1;
    s32 x30c_ = -1;
    s32 x310_ = -1;
    u32 x314_ = 0;
    u32 x318_ = 0;
    u32 x31c_ = 0;
    u32 x320_ = 0;
    u32 x324_ = 0;
    u8 x334_ = 0;
    u8 x344_ = 0;
    u32 x348_ = 0;
    float x34c_ = 0.f;
    u32 x350_;
    union
    {
        struct
        {
            bool x354_24_ : 1;
            bool x354_25_ : 1;
            bool x354_26_ : 1;
            bool x354_27_ : 1;
            bool x354_28_ : 1;
            bool x354_29_ : 1;
            bool x354_30_ : 1;
            bool x354_31_ : 1;
            bool x355_24_ : 1;
        };
        u8 x355_dummy = 0;
    };

public:
    CScriptPlayerActor(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, const CAnimRes&,
                       CModelData&&, const zeus::CAABox&, bool, const CMaterialList&, float, float,
                       const CHealthInfo&, const CDamageVulnerability&, const CActorParameters&, bool, bool, u32, u32);

    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void SetActive(bool active);
    void PreRender(CStateManager &, const zeus::CFrustum &);
    void AddToRenderer(const zeus::CFrustum &, const CStateManager &) const;
    void Render(const CStateManager &mgr) const;

};
}

#endif // __URDE_CSCRIPTPLAYERACTOR_HPP__
