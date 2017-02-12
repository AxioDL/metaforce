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
    };

private:
    TUniqueId x34_ = kInvalidUniqueId;
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
            bool x54_26_ : 1;
            bool x54_27_ : 1;
            bool x54_28_ : 1;
            bool x54_29_ : 1;
            bool x54_30_ : 1;
            bool x54_31_ : 1;
            bool x55_24_ : 1;
        };
        u32 _dummy = 0;
    };

public:
    CScriptColorModulate(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CColor&, const zeus::CColor&,
                         EBlendMode, float, float, bool, bool, bool, bool, bool, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
    void Think(float, CStateManager &);
    zeus::CColor CalculateFlags(const zeus::CColor&) const;
    void SetTargetFlags(CStateManager&, const CModelFlags&);
    static void FadeOutHelper(CStateManager&, TUniqueId, float);
    static void FadeInHelper(CStateManager&, TUniqueId, float);
    void End(CStateManager&);
};
}
#endif // __URDE_CSCRIPTCOLORMODULATE_HPP__
