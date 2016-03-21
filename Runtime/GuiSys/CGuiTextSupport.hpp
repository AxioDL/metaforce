#ifndef __URDE_CGUITEXTSUPPORT_HPP__
#define __URDE_CGUITEXTSUPPORT_HPP__

#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CTextRenderBuffer.hpp"
#include "optional.hpp"
#include <string>

namespace urde
{
class CSimplePool;
class CRasterFont;
class CTextRenderBuffer;

enum class EJustification
{
    Left = 0,
    Center,
    Right,
    Full,
    NLeft,
    NCenter,
    NRight,
    Seven,
    Eight,
    Nine
};

enum class EVerticalJustification
{
    Top = 0,
    Center,
    Bottom,
    Full,
    NTop,
    NCenter,
    NBottom,
    Seven,
    Eight,
    Nine
};

enum class EColorType
{
    Main,
    Outline,
    Geometry,
    Foreground,
    Four
};

enum class ETextDirection
{
    Horizontal,
    Vertical
};

using CTextColor = zeus::CColor;

class CGuiTextProperties
{
    bool x0_a;
    bool x1_b;
    bool x2_c;
    EJustification x4_justification;
    EVerticalJustification x8_vertJustification;
public:
    CGuiTextProperties(bool a, bool b, bool c, EJustification justification,
                       EVerticalJustification vertJustification)
        : x0_a(a), x1_b(b), x2_c(c), x4_justification(justification),
          x8_vertJustification(vertJustification) {}
};

class CGuiTextSupport
{
    u32 x4_ = 0;
    u32 x8_ = 0;
    CGuiTextProperties x10_props;
    zeus::CColor x1c_;
    zeus::CColor x20_;
    zeus::CColor x24_;
    s32 x28_;
    s32 x2c_;
    float x30_ = 0.f;
    std::vector<u32> x34_;
    s32 x44_ = 0;
    float x48_ = 0.1f;
    float x4c_ = 10.0f;
    TResId x50_fontId;
    std::experimental::optional<CTextRenderBuffer> x54_renderBuf;
    bool x2ac_ = false;
    s32 x2b4_ = 0;
    s32 x2b8_ = 0;
    s32 x2bc_ = 0;
    TLockedToken<CRasterFont> x2c0_font;
public:
    CGuiTextSupport(TResId fontId, const CGuiTextProperties& props,
                    const zeus::CColor& col1, const zeus::CColor& col2,
                    const zeus::CColor& col3, s32, s32, CSimplePool*);
    void GetCurrentAnimationOverAge() const;
    int GetNumCharsPrinted() const;
    int GetTotalAnimationTime() const;
    void SetTypeWriteEffectOptions(bool, float, float);
    void Update(float dt);
    void ClearBuffer();
    void CheckAndRebuildTextRenderBuffer();
    void Render() const;
    void SetGeometryColor(const zeus::CColor& col);
    void SetOutlineColor(const zeus::CColor& col);
    void SetFontColor(const zeus::CColor& col);
    void AddText(const std::wstring& str);
    void SetText(const std::wstring& str);
    void SetText(const std::string& str);
    bool GetIsTextSupportFinishedLoading() const;
};

}

#endif // __URDE_CGUITEXTSUPPORT_HPP__
