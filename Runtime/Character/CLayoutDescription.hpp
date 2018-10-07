#pragma once

#include "optional.hpp"
#include "CToken.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CCharLayoutInfo;

class CLayoutDescription
{
public:
    class CScaledLayoutDescription
    {
        TLockedToken<CCharLayoutInfo> x0_layoutToken;
        float xc_scale;
        std::experimental::optional<zeus::CVector3f> x10_scaleVec;
    public:
        const TLockedToken<CCharLayoutInfo>& ScaledLayout() const {return x0_layoutToken;}
        float GlobalScale() const {return xc_scale;}
        const std::experimental::optional<zeus::CVector3f>& GetScaleVec() const {return x10_scaleVec;}
    };
private:
    TLockedToken<CCharLayoutInfo> x0_layoutToken;
    std::experimental::optional<CScaledLayoutDescription> xc_scaled;
public:
    CLayoutDescription(const TLockedToken<CCharLayoutInfo>& token)
    : x0_layoutToken(token) {}

    const std::experimental::optional<CScaledLayoutDescription>&
    GetScaledLayoutDescription() const {return xc_scaled;}

    const TLockedToken<CCharLayoutInfo>& GetCharLayoutInfo() const {return x0_layoutToken;}
    bool UsesScale() const { return bool(xc_scaled); }
    const TLockedToken<CCharLayoutInfo>& ScaledLayout() const
    {
        if (UsesScale())
            return xc_scaled->ScaledLayout();
        return x0_layoutToken;
    }
};

}

