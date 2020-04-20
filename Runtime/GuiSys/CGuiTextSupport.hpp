#pragma once

#include <list>
#include <optional>
#include <string>
#include <vector>

#include "Runtime/CSaveWorld.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CTextRenderBuffer.hpp"

#include <zeus/CColor.hpp>

namespace urde {
class CSimplePool;
class CTextExecuteBuffer;
class CTextRenderBuffer;

enum class EJustification { Left = 0, Center, Right, Full, NLeft, NCenter, NRight, LeftMono, CenterMono, RightMono };

enum class EVerticalJustification {
  Top = 0,
  Center,
  Bottom,
  Full,
  NTop,
  NCenter,
  NBottom,
  TopMono,
  CenterMono,
  RightMono
};

enum class ETextDirection { Horizontal, Vertical };

class CGuiTextProperties {
  friend class CGuiTextSupport;
  bool x0_wordWrap;
  bool x1_horizontal;
  EJustification x4_justification;
  EVerticalJustification x8_vertJustification;
  const std::vector<std::pair<CAssetId, CAssetId>>* xc_txtrMap;

public:
  CGuiTextProperties(bool wordWrap, bool horizontal, EJustification justification,
                     EVerticalJustification vertJustification,
                     const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap = nullptr)
  : x0_wordWrap(wordWrap)
  , x1_horizontal(horizontal)
  , x4_justification(justification)
  , x8_vertJustification(vertJustification)
  , xc_txtrMap(txtrMap) {}
};

class CGuiTextSupport {
  friend class CGuiTextPane;
  std::u16string x0_string;
  float x10_curTimeMod900 = 0.f;
  CGuiTextProperties x14_props;
  zeus::CColor x24_fontColor;
  zeus::CColor x28_outlineColor;
  zeus::CColor x2c_geometryColor;
  bool x30_imageBaseline = false;
  s32 x34_extentX;
  s32 x38_extentY;
  float x3c_curTime = 0.f;
  std::vector<std::pair<float, int>> x40_primStartTimes;
  bool x50_typeEnable = false;
  float x54_chFadeTime = 0.1f;
  float x58_chRate = 10.0f;
  CAssetId x5c_fontId;
  CGuiWidget::EGuiModelDrawFlags m_drawFlags;
  std::optional<CTextRenderBuffer> x60_renderBuf;
  std::vector<CToken> x2bc_assets;
  TLockedToken<CRasterFont> x2cc_font;
  std::pair<zeus::CVector2i, zeus::CVector2i> x2dc_oneBufBounds;

  std::list<CTextRenderBuffer> x2ec_renderBufferPages;
  int x304_pageCounter = 0;
  bool x308_multipageFlag = false;

  CTextRenderBuffer* GetCurrentPageRenderBuffer();
  const CTextRenderBuffer* GetCurrentPageRenderBuffer() const;

  bool _GetIsTextSupportFinishedLoading();

public:
  CGuiTextSupport(CAssetId fontId, const CGuiTextProperties& props, const zeus::CColor& fontCol,
                  const zeus::CColor& outlineCol, const zeus::CColor& geomCol, s32 extX, s32 extY, CSimplePool* store,
                  CGuiWidget::EGuiModelDrawFlags drawFlags);
  float GetCurrentAnimationOverAge() const;
  float GetNumCharsTotal() const;
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

  void Render();
  void SetGeometryColor(const zeus::CColor& col);
  void SetOutlineColor(const zeus::CColor& col);
  void SetFontColor(const zeus::CColor& col);
  void AddText(std::u16string_view str);
  void SetText(std::u16string_view str, bool multipage = false);
  void SetText(std::string_view str, bool multipage = false);
  void SetJustification(EJustification j);
  void SetVerticalJustification(EVerticalJustification j);
  void SetImageBaseline(bool b);
  bool GetIsTextSupportFinishedLoading();
  float GetCurTime() const { return x3c_curTime; }
  void SetCurTime(float t) { x3c_curTime = t; }
  std::u16string_view GetString() const { return x0_string; }
  void SetControlTXTRMap(const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap);
  int GetPageCounter() const { return x304_pageCounter; }
  int GetTotalPageCount();
  void SetPage(int page);
};

} // namespace urde
