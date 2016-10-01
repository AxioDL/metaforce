#ifndef __URDE_CLIGHT_HPP__
#define __URDE_CLIGHT_HPP__

#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"

namespace urde
{

enum class ELightType
{
    Spot = 0,
    Point = 1,
    Directional = 2,
    LocalAmbient = 3,
    Custom = 4,
};
enum class EFalloffType
{
    Constant,
    Linear,
    Quadratic
};

class CLight
{
    friend class CGuiLight;
    friend class CBooModel;
    friend class CBooRenderer;
    friend class CGameLight;

    zeus::CVector3f x0_pos;
    zeus::CVector3f xc_dir;
    zeus::CColor x18_color;
    ELightType x1c_type;
    float x20_spotCutoff;
    float x24_distC;
    float x28_distL;
    float x2c_distQ;
    float x30_angleC;
    float x34_angleL;
    float x38_angleQ;
    u32 x3c_ = 0;
    u32 x40_loadedIdx = 0;
    float x44_cachedRadius;
    float x48_cachedIntensity;
    bool x4c_24_intensityDirty : 1;
    bool x4c_25_radiusDirty : 1;

    float CalculateLightRadius() const;

public:
    CLight(const zeus::CVector3f& pos,
           const zeus::CVector3f& dir,
           const zeus::CColor& color,
           float distC, float distL, float distQ,
           float angleC, float angleL, float angleQ);

    CLight(ELightType type,
           const zeus::CVector3f& pos,
           const zeus::CVector3f& dir,
           const zeus::CColor& color,
           float cutoff);

    void SetPosition(const zeus::CVector3f& pos)
    {
        x0_pos = pos;
    }

    const zeus::CVector3f& GetPosition() const { return x0_pos; }

    void SetDirection(const zeus::CVector3f& dir)
    {
        xc_dir = dir;
    }

    const zeus::CVector3f& GetDirection() const { return xc_dir; }

    void SetColor(const zeus::CColor& col)
    {
        x18_color = col;
        x4c_24_intensityDirty = true;
        x4c_25_radiusDirty = true;
    }

    void SetAttenuation(float constant, float linear, float quadratic)
    {
        x24_distC = constant;
        x28_distL = linear;
        x2c_distQ = quadratic;
        x4c_24_intensityDirty = true;
        x4c_25_radiusDirty = true;
    }

    void SetAngleAttenuation(float constant, float linear, float quadratic)
    {
        x30_angleC = constant;
        x34_angleL = linear;
        x38_angleQ = quadratic;
        x4c_24_intensityDirty = true;
        x4c_25_radiusDirty = true;
    }

    float GetRadius() const
    {
        if (x4c_25_radiusDirty)
        {
            const_cast<CLight*>(this)->x44_cachedRadius = CalculateLightRadius();
            const_cast<CLight*>(this)->x4c_25_radiusDirty = false;
        }
        return x44_cachedRadius;
    }

    ELightType GetType() const { return x1c_type; }

    float GetIntensity() const;
    const zeus::CColor& GetColor() const { return x18_color; }

    static CLight BuildDirectional(const zeus::CVector3f& dir, const zeus::CColor& color);
    static CLight BuildSpot(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                            const zeus::CColor& color, float angle);
    static CLight BuildPoint(const zeus::CVector3f& pos, const zeus::CColor& color);
    static CLight BuildCustom(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                              const zeus::CColor& color,
                              float distC, float distL, float distQ,
                              float angleC, float angleL, float angleQ);
    static CLight BuildLocalAmbient(const zeus::CVector3f& pos, const zeus::CColor& color);
};

}

#endif // __URDE_CLIGHT_HPP__
