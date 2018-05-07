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
        Zero,
        One,
        Two,
        Three,
        Four,
    };

private:
    TUniqueId x34_parent = kInvalidUniqueId;
    u32 x38_ = 0;
    float x3c_;
    zeus::CColor x40_;
    zeus::CColor x44_;
    EBlendMode x48_blendMode;
    float x4c_;
    float x50_;
    union
    {
        struct
        {
            bool x54_24_ : 1;
            bool x54_25_ : 1;
            bool x54_26_depthEqual : 1;
            bool x54_27_depthUpdate : 1;
            bool x54_28_ : 1;
            bool x54_29_ : 1;
            bool x54_30_ : 1;
            bool x54_31_ : 1;
            bool x55_24_ : 1;
        };
        u32 _dummy = 0;
    };

public:
    CScriptColorModulate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CColor&, const zeus::CColor&,
                         EBlendMode, float, float, bool, bool, bool, bool, bool, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    void Think(float, CStateManager &);
    CModelFlags CalculateFlags(const zeus::CColor&) const;
    void SetTargetFlags(CStateManager&, const CModelFlags&);
    static TUniqueId FadeOutHelper(CStateManager&, TUniqueId, float);
    static TUniqueId FadeInHelper(CStateManager&, TUniqueId, float);
    void End(CStateManager&);
};
}
#endif // __URDE_CSCRIPTCOLORMODULATE_HPP__
