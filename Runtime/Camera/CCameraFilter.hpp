#ifndef __URDE_CCAMERAFILTER_HPP__
#define __URDE_CCAMERAFILTER_HPP__

#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"

namespace urde
{
class CTexture;

class CCameraFilterPass
{
public:
    enum class EFilterType
    {
        Passthru,
        Multiply,
        Invert,
        Add,
        Subtract,
        Blend,
        Widescreen,
        SceneAdd,
        NoColor
    };
    enum class EFilterShape
    {
        Fullscreen,
        FullscreenHalvesLeftRight,
        FullscreenHalvesTopBottom,
        FullscreenQuarters,
        CinemaBars,
        ScanLinesEven,
        ScanLinesOdd,
        RandomStatic,
        CookieCutterDepthRandomStatic
    };
private:
    static void DrawFilter(EFilterType type, EFilterShape shape, const zeus::CColor& color,
                           const CTexture* tex, float uvScale);
    static void DrawFilterShape(EFilterShape shape, const zeus::CColor& color,
                                const CTexture* tex, float uvScale);
public:
    void SetFilter(EFilterType type, EFilterShape shape, float, const zeus::CColor& color, u32) {}
    void DisableFilter(float) {}

};

class CCameraBlurPass
{
public:
    enum class EBlurType
    {
        NoBlur,
        LoBlur,
        HiBlur,
        Xray
    };
private:
    TLockedToken<CTexture> x0_paletteTex;
    EBlurType x10_curType = EBlurType::NoBlur;
    EBlurType x14_endType = EBlurType::NoBlur;
    float x18_endValue = 0.f;
    float x1c_curValue = 0.f;
    float x20_startValue = 0.f;
    float x24_totalTime = 0.f;
    float x28_remainingTime = 0.f;
    //bool x2c_usePersistent = false;
    //bool x2d_noPersistentCopy = false;
    //u32 x30_persistentBuf = 0;

    void Draw();
    void Update(float dt);
    void SetBlur(EBlurType type, float amount, float duration);
    void DisableBlur(float duration);
};

}

#endif // __URDE_CCAMERAFILTER_HPP__
