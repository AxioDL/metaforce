#ifndef _CGUITEXTSUPPORT
#define _CGUITEXTSUPPORT

#include "rstl/list.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Text/CTextRenderBuffer.hpp"
#include "rstl/optional_object.hpp"

#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Text/TextCommon.hpp"

#include "rstl/pair.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CColor;
class CGuiFrame;
class CTextExecuteBuffer;
class CTextParser;
class CSimplePool;

class CGuiTextProperties {
  friend class CGuiTextSupport;

public:
  CGuiTextProperties(const bool wordWrap, const bool horizontal, const EJustification justification,
                     const EVerticalJustification vertJustification,
                     const rstl::vector< rstl::pair< CAssetId, CAssetId > >* txtrMap = nullptr);

  bool GetIsWrap() const { return x0_wordWrap; }
  bool GetIsLeftToRight() const { return x1_horizontal; }
  EJustification GetJustification() const { return x4_justification; }
  EVerticalJustification GetVerticalJustification() const { return x8_vertJustification; }

private:
  bool x0_wordWrap;
  bool x1_horizontal;
  EJustification x4_justification;
  EVerticalJustification x8_vertJustification;
  const rstl::vector< rstl::pair< CAssetId, CAssetId > >* xc_txtrMap;
};

class CGuiTextSupport {
  static CTextExecuteBuffer* gpExecBuf;
  static CTextParser* gpTextParser;

public:
  CGuiTextSupport(CAssetId fontId, const CGuiTextProperties& props, const CColor& fontCol,
                  const CColor& outlineCol, const CColor& geomCol, int extX, int extY,
                  CSimplePool* store);
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CGuiTextSupport(CAssetId fontId, int extX, int extY, const CGuiTextProperties& props,
                  const CColor& fontCol, const CColor& outlineCol, const CColor& geomCol,
                  CSimplePool* store);
#endif
  ~CGuiTextSupport();

  void SetText(const rstl::wstring&, bool multipage = false);
  void SetText(const rstl::string&, bool multipage = false);
  void AddText(const rstl::wstring& str);
  void SetWordWrap(bool wordWrap);
  void SetJustification(EJustification just);
  void SetVerticalJustification(EVerticalJustification just);
  void SetControlTXTRMap(const rstl::vector< rstl::pair< CAssetId, CAssetId > >* txtrMap);
  void Update(float dt);
  void CheckAndRebuildTextBuffer() const;
  bool CheckAndRebuildRenderBuffer() const;
  const CTextRenderBuffer* GetCurrentPageRenderBuffer() const;
  CTextRenderBuffer* GetCurrentPageRenderBuffer() {
    return const_cast< CTextRenderBuffer* >(
        static_cast< const CGuiTextSupport* >(this)->GetCurrentPageRenderBuffer());
  }
  float GetCurrentAnimationOverAge() const;
  int GetTotalPageCount();
  void SetPage(int page);
  int GetPageCounter() const { return x304_pageCounter; }
  void ClearRenderBuffer();
  void SetImageBaseline(bool baseline);
  void SetTypeWriteEffectOptions(bool enable, float fadeTime, float rate);
  void SetGeometryColor(const CColor& col);
  void SetOutlineColor(const CColor& col);
  void SetFontColor(const CColor& col);
  void Render() const;
  const rstl::pair< CVector2i, CVector2i >& GetBounds();
  bool GetIsTextSupportFinishedLoading() const;

  void SetExtentX(int extent) {
    x34_extentX = extent;
    ClearRenderBuffer();
  }
  void SetExtentY(int extent) {
    x38_extentY = extent;
    ClearRenderBuffer();
  }

  float GetNumCharsTotal() const;
  float GetNumCharactersPrinted() const;
  float GetTotalAnimationTime() const;
  bool IsAnimationDone() const { return x3c_curTime >= GetTotalAnimationTime(); }
  CAssetId GetFontID() const { return x5c_fontId; }
  int GetTextBoundingWidth() const { return x34_extentX; }
  int GetTextBoundingHeight() const { return x38_extentY; }
  const rstl::wstring& GetText() const { return x0_string; }
  float GetCurTime() const { return x3c_curTime; }
  void SetCurTime(float t) { x3c_curTime = t; }

  static void Initialize(CTextExecuteBuffer* buf, CTextParser* parser) {
    gpExecBuf = buf;
    gpTextParser = parser;
  }

private:
  bool _GetIsTextSupportFinishedLoading() const;

  rstl::wstring x0_string;
  float x10_curTimeMod900;
  CGuiTextProperties x14_props;
  CColor x24_fontColor;
  CColor x28_outlineColor;
  CColor x2c_geometryColor;
  bool x30_imageBaseline;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  int x34_extraCharacterSpace;
  int x38_extraLineSpace;
#endif
  int x34_extentX;
  int x38_extentY;
  float x3c_curTime;
  rstl::vector< rstl::pair< float, int > > x40_primStartTimes;
  bool x50_typeEnable;
  float x54_chFadeTime;
  float x58_chRate;
  CAssetId x5c_fontId;
  mutable rstl::optional_object< CTextRenderBuffer > x60_renderBuf;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  bool xbd8_;
#endif
  mutable rstl::vector< CToken > x2bc_assets;
  rstl::optional_object< TLockedToken< CRasterFont > > x2cc_font;
  mutable rstl::pair< CVector2i, CVector2i > x2dc_oneBufBounds;
  mutable rstl::list< CTextRenderBuffer > x2ec_renderBufferPages;
  int x304_pageCounter;
  bool x308_multipageFlag;
};

CHECK_SIZEOF(CGuiTextProperties, 0x10)
CHECK_SIZEOF(CGuiTextSupport, (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0xc2c : 0x30c))

#endif // _CGUITEXTSUPPORT
