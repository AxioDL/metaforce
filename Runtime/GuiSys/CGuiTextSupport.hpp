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
    Background
};

enum class ETextDirection
{
    Horizontal,
    Vertical
};

class CGuiTextProperties
{
    friend class CGuiTextSupport;
    bool x0_wordWrap;
    bool x1_vertical;
    bool x2_c;
    EJustification x4_justification;
    EVerticalJustification x8_vertJustification;
public:
    CGuiTextProperties(bool a, bool b, bool c, EJustification justification,
                       EVerticalJustification vertJustification)
        : x0_wordWrap(a), x1_vertical(b), x2_c(c), x4_justification(justification),
          x8_vertJustification(vertJustification) {}
};

class CGuiTextSupport
{
    std::wstring x0_string;
    CGuiTextProperties x10_props;
    zeus::CColor x1c_fontColor;
    zeus::CColor x20_outlineColor;
    zeus::CColor x24_geometryColor;
    s32 x28_padX;
    s32 x2c_padY;
    float x30_curTime = 0.f;
    std::vector<std::pair<float, int>> x34_primStartTimes;
    bool x44_typeEnable = false;
    float x48_chFadeTime = 0.1f;
    float x4c_chRate = 10.0f;
    TResId x50_fontId;
    std::experimental::optional<CTextRenderBuffer> x54_renderBuf;
    bool x2ac_active = false;
    std::vector<CToken> x2b0_assets;
    TLockedToken<CRasterFont> x2c0_font;
public:
    CGuiTextSupport(TResId fontId, const CGuiTextProperties& props,
                    const zeus::CColor& col1, const zeus::CColor& col2,
                    const zeus::CColor& col3, s32 padX, s32 padY, CSimplePool* store);
    float GetCurrentAnimationOverAge() const;
    float GetNumCharsPrinted() const;
    float GetTotalAnimationTime() const;
    void SetTypeWriteEffectOptions(bool enable, float chFadeTime, float chRate);
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
