#ifndef __URDE_CSCRIPTSHADOWPROJECTOR_HPP__
#define __URDE_CSCRIPTSHADOWPROJECTOR_HPP__

#include "World/CActor.hpp"

namespace urde
{
class CProjectedShadow;
class CScriptShadowProjector : public CActor
{
    float xe8_scale;
    zeus::CVector3f xec_offset;
    float xf8_zOffsetAdjust;
    float xfc_opacity;
    float x100_opacityRecip;
    TUniqueId x104_target;
    std::unique_ptr<CProjectedShadow> x108_projectedShadow;
    u32 x10c_textureSize;

    union {
        struct
        {
            bool x110_24_persistent : 1;
            bool x110_25_shadowInvalidated : 1;
        };
        u8 x110_dummy = 0;
    };

public:
    CScriptShadowProjector(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, bool,
                           const zeus::CVector3f&, bool, float, float, float, float, s32);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager &);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void PreRender(CStateManager &, const zeus::CFrustum &);
    void AddToRenderer(const zeus::CFrustum &, const CStateManager &) const {}
    void CreateProjectedShadow();
};
}

#endif // __URDE_CSCRIPTSHADOWPROJECTOR_HPP__
