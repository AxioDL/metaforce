#ifndef __URDE_CSCRIPTCOLORMODULATE_HPP__
#define __URDE_CSCRIPTCOLORMODULATE_HPP__

#include "CEntity.hpp"
#include "zeus/CColor.hpp"

namespace urde
{
struct CModelFlags;
class CScriptColorModulate : public CEntity
{
public:
    enum class EBlendMode
    {
        Alpha,
        Additive,
        Additive2,
        Opaque,
        Opaque2,
    };
    enum class EFadeState
    {
        A2B,
        B2A
    };

private:
    TUniqueId x34_parent = kInvalidUniqueId;
    EFadeState x38_fadeState = EFadeState::A2B;
    float x3c_curTime = 0.f;
    zeus::CColor x40_colorA;
    zeus::CColor x44_colorB;
    EBlendMode x48_blendMode;
    float x4c_timeA2B;
    float x50_timeB2A;
    union
    {
        struct
        {
            bool x54_24_doReverse : 1;
            bool x54_25_resetTargetWhenDone : 1;
            bool x54_26_depthCompare : 1;
            bool x54_27_depthUpdate : 1;
            bool x54_28_depthBackwards : 1;
            bool x54_29_reversing : 1;
            bool x54_30_enable : 1;
            bool x54_31_dieOnEnd : 1;
            bool x55_24_isFadeOutHelper : 1;
        };
        u32 _dummy = 0;
    };

public:
    CScriptColorModulate(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CColor& colorA, const zeus::CColor& colorB,
                         EBlendMode blendMode, float timeA2B, float timeB2A, bool doReverse,
                         bool resetTargetWhenDone, bool depthCompare, bool depthUpdate,
                         bool depthBackwards, bool active);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    void Think(float, CStateManager &);
    CModelFlags CalculateFlags(const zeus::CColor&) const;
    void SetTargetFlags(CStateManager&, const CModelFlags&);
    static TUniqueId FadeOutHelper(CStateManager& mgr, TUniqueId obj, float fadetime);
    static TUniqueId FadeInHelper(CStateManager& mgr, TUniqueId obj, float fadetime);
    void End(CStateManager&);
};
}
#endif // __URDE_CSCRIPTCOLORMODULATE_HPP__
