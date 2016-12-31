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
class CTextExecuteBuffer;

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
    bool x1_multiline;
    EJustification x4_justification;
    EVerticalJustification x8_vertJustification;
    ETextDirection xc_direction;
public:
    CGuiTextProperties(bool wordWrap, bool multiline, EJustification justification,
                       EVerticalJustification vertJustification,
                       ETextDirection dir=ETextDirection::Horizontal)
        : x0_wordWrap(wordWrap), x1_multiline(multiline), x4_justification(justification),
          x8_vertJustification(vertJustification), xc_direction(dir) {}
};

class CGuiTextSupport
{
    friend class CGuiTextPane;
    std::wstring x0_string;
    float x10_curTimeMod900 = 0.f;
    CGuiTextProperties x14_props;
    zeus::CColor x24_fontColor;
    zeus::CColor x28_outlineColor;
    zeus::CColor x2c_geometryColor;
    s32 x34_extentX;
    s32 x38_extentY;
    float x3c_curTime = 0.f;
    std::vector<std::pair<float, int>> x40_primStartTimes;
    bool x50_typeEnable = false;
    float x54_chFadeTime = 0.1f;
    float x58_chRate = 10.0f;
    ResId x5c_fontId;
    std::experimental::optional<CTextRenderBuffer> x60_renderBuf;
    std::vector<CToken> x2bc_assets;
    TLockedToken<CRasterFont> x2cc_font;
    std::pair<zeus::CVector2i, zeus::CVector2i> x2dc_oneBufBounds;

    std::list<CTextRenderBuffer> x2ec_renderBufferPages;
    u32 x300_ = 0;
    u32 x304_pageCounter = 0;
    bool x308_multipageFlag = false;

    CTextRenderBuffer* GetCurrentPageRenderBuffer() const;
    bool _GetIsTextSupportFinishedLoading() const;

public:
    CGuiTextSupport(ResId fontId, const CGuiTextProperties& props,
                    const zeus::CColor& fontCol, const zeus::CColor& outlineCol,
                    const zeus::CColor& geomCol, s32 extX, s32 extY, CSimplePool* store);
    float GetCurrentAnimationOverAge() const;
    float GetNumCharsPrinted() const;
    float GetTotalAnimationTime() const;
    bool IsAnimationDone() const;
    void SetTypeWriteEffectOptions(bool enable, float chFadeTime, float chRate);
    void Update(float dt);
    void ClearRenderBuffer();
    void CheckAndRebuildTextBuffer();
    bool CheckAndRebuildRenderBuffer();
    const std::pair<zeus::CVector2i, zeus::CVector2i>& GetBounds();
    void AutoSetExtent();

    void Render() const;
    void SetGeometryColor(const zeus::CColor& col);
    void SetOutlineColor(const zeus::CColor& col);
    void SetFontColor(const zeus::CColor& col);
    void AddText(const std::wstring& str);
    void SetText(const std::wstring& str, bool multipage=false);
    void SetText(const std::string& str, bool multipage=false);
    bool GetIsTextSupportFinishedLoading() const;
};

}

#endif // __URDE_CGUITEXTSUPPORT_HPP__
