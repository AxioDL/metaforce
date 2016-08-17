#ifndef __URDE_CCAMERAFILTER_HPP__
#define __URDE_CCAMERAFILTER_HPP__

#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"

namespace urde
{
class CTexture;

class CCameraFilterPass
{
public:
    enum class EFilterType
    {
        None,
        ColorMultiply,
        InvertDst,
        AdditiveAlpha,
        Subtractive,
        AlphaBlended,
        None2,
        AdditiveDestColor,
        NoColorWrite
    };
    enum class EFilterShape
    {
        QuadA,
        QuadB,
        QuadC,
        QuadQuarters,
        WideScreen,
        ScanLinesA,
        ScanLinesB,
        RandomStaticA,
        RandomStaticB
    };
private:
public:
    void SetFilter(EFilterType type, EFilterShape shape, float, const zeus::CColor& color, u32) {}
    void DisableFilter(float) {}
    static void DrawFilter(EFilterType type, EFilterShape shape, const zeus::CColor& color,
                           const CTexture* tex, float uvScale);
    static void DrawFilterShape(EFilterShape shape, const zeus::CColor& color,
                                const CTexture* tex, float uvScale);
};

class CCameraBlurPass
{
};

}

#endif // __URDE_CCAMERAFILTER_HPP__
