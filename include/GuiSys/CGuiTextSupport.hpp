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

  bool GetIsWrap() const { return mWordWrap; }
  bool GetIsLeftToRight() const { return mHorizontal; }
  EJustification GetJustification() const { return mJustification; }
  EVerticalJustification GetVerticalJustification() const { return mVertJustification; }

private:
  bool mWordWrap;
  bool mHorizontal;
  EJustification mJustification;
  EVerticalJustification mVertJustification;
  const rstl::vector< rstl::pair< CAssetId, CAssetId > >* mTxtrMap;
};

class CGuiTextSupport {
  static CTextExecuteBuffer* gpExecBuf;
  static CTextParser* gpTextParser;

public:
  CGuiTextSupport(CAssetId fontId, const CGuiTextProperties& props, const CColor& fontCol,
                  const CColor& outlineCol, const CColor& geomCol, int extX, int extY,
                  CSimplePool* store);
#if VERSION >= VERSION_GM8P_00
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
  int GetPageCounter() const { return mPageCounter; }
  void ClearRenderBuffer();
  void SetImageBaseline(bool baseline);
  void SetTypeWriteEffectOptions(bool enable, float fadeTime, float rate);
  void SetGeometryColor(const CColor& col);
  const CColor& GetGeometryColor() const { return mGeometryColor; }
  void SetOutlineColor(const CColor& col);
  void SetFontColor(const CColor& col);
  void Render() const;
  const rstl::pair< CVector2i, CVector2i >& GetBounds();
  bool GetIsTextSupportFinishedLoading() const;

  void SetExtentX(int extent) {
    mExtentX = extent;
    ClearRenderBuffer();
  }
  void SetExtentY(int extent) {
    mExtentY = extent;
    ClearRenderBuffer();
  }

  float GetNumCharsTotal() const;
  float GetNumCharactersPrinted() const;
  float GetTotalAnimationTime() const;
  bool IsAnimationDone() const { return mCurTime >= GetTotalAnimationTime(); }
  CAssetId GetFontID() const { return mFontId; }
  int GetTextBoundingWidth() const { return mExtentX; }
  int GetTextBoundingHeight() const { return mExtentY; }
  const rstl::wstring& GetText() const { return mString; }
  float GetCurTime() const { return mCurTime; }
  void SetCurTime(float t) { mCurTime = t; }

  static void Initialize(CTextExecuteBuffer* buf, CTextParser* parser) {
    gpExecBuf = buf;
    gpTextParser = parser;
  }

private:
  bool _GetIsTextSupportFinishedLoading() const;

  rstl::wstring mString;
  float mCurTimeMod900;
  CGuiTextProperties mProps;
  CColor mFontColor;
  CColor mOutlineColor;
  CColor mGeometryColor;
  bool mImageBaseline;
#if VERSION >= VERSION_GM8P_00
  int mExtraCharacterSpace;
  int mExtraLineSpace;
#endif
  int mExtentX;
  int mExtentY;
  float mCurTime;
  rstl::vector< rstl::pair< float, int > > mPrimStartTimes;
  bool mTypeEnable;
  float mChFadeTime;
  float mChRate;
  CAssetId mFontId;
  mutable rstl::optional_object< CTextRenderBuffer > mRenderBuf;
#if VERSION >= VERSION_GM8P_00
  bool xbd8_;
#endif
  mutable rstl::vector< CToken > mAssets;
  rstl::optional_object< TLockedToken< CRasterFont > > mFont;
  mutable rstl::pair< CVector2i, CVector2i > mOneBufBounds;
  mutable rstl::list< CTextRenderBuffer > mRenderBufferPages;
  int mPageCounter;
  bool mMultipageFlag;
};

CHECK_SIZEOF(CGuiTextProperties, 0x10)
CHECK_SIZEOF(CGuiTextSupport, (VERSION >= VERSION_GM8P_00 ? 0xc2c : 0x30c))

#endif // _CGUITEXTSUPPORT
