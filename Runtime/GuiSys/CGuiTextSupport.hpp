#ifndef __URDE_CGUITEXTSUPPORT_HPP__
#define __URDE_CGUITEXTSUPPORT_HPP__

#include "zeus/CColor.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"
#include <string>

namespace urde
{
class CSimplePool;
class CRasterFont;

enum class EJustification
{
    Zero = 0,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine
};

enum class EVerticalJustification
{
    Zero = 0,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine
};

enum class EColorType
{
    Zero,
    One,
    Two,
    Three,
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
    TResId x50_fontId;
    TLockedToken<CRasterFont> x2c0_font;
public:
    CGuiTextSupport(TResId fontId, const CGuiTextProperties& props,
                    const zeus::CColor& col1, const zeus::CColor& col2,
                    const zeus::CColor& col3, int, int, CSimplePool*);
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
