#ifndef _CRASTERFONT
#define _CRASTERFONT

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

#include "string.h"

class CTexture;
class CDrawStringOptions;
class CTextRenderBuffer;
class IObjectStore;

class CFontInfo {
public:
  CFontInfo(bool a, bool b, int c, int fontSize, const char* const name)
  : x0_(a), x1_(b), x4_(c), mFontSize(fontSize) {
    strcpy(mName, name);
  }

private:
  bool x0_;
  bool x1_;
  int x4_;
  int mFontSize;
  char mName[64];
};

CHECK_SIZEOF(CFontInfo, 0x4c)

class CKernPair {
public:
  CKernPair(const wchar_t first, const wchar_t second, const int howMuch) : mHowMuch(howMuch) {
    mSecond = second;
    mFirst = first;
  }

  wchar_t GetFirst() const { return mFirst; }
  wchar_t GetSecond() const { return mSecond; }
  int GetHowMuch() const { return mHowMuch; }

private:
  wchar_t mFirst;
  wchar_t mSecond;
  int mHowMuch;
};
// TODO: breaks clangd
// CHECK_SIZEOF(CKernPair, 0x8)

class CGlyph {
public:
  CGlyph(const int a, const int b, const int c, const float startU, const float startV,
         const float endU, const float endV, const int cellWidth, const int cellHeight,
         const int baseline, const int kernStart)
  : mA(a)
  , mB(b)
  , mC(c)
  , mStartU(startU)
  , mStartV(startV)
  , mEndU(endU)
  , mEndV(endV)
  , mCellWidth(cellWidth)
  , mCellHeight(cellHeight)
  , mBaseline(baseline)
  , mKernStart(kernStart) {}

  short GetA() const { return mA; }
  short GetB() const { return mB; }
  short GetC() const { return mC; }
  float GetStartU() const { return mStartU; }
  float GetStartV() const { return mStartV; }
  float GetEndU() const { return mEndU; }
  float GetEndV() const { return mEndV; }
  short GetCellWidth() const { return mCellWidth; }
  short GetCellHeight() const { return mCellHeight; }
  short GetBaseLine() const { return mBaseline; }
  int GetKernStart() const { return mKernStart; }

private:
  short mA;
  short mB;
  short mC;
  float mStartU;
  float mStartV;
  float mEndU;
  float mEndV;
  short mCellWidth;
  short mCellHeight;
  short mBaseline;
  short mKernStart;
};

CHECK_SIZEOF(CGlyph, 0x20)

enum EFontMode {
  kFM_None = -1,
  kFM_OneLayer,
  kFM_OneLayerOutline,
  kFM_FourLayers,
  kFM_TwoLayersOutline,
  kFM_TwoLayers,
};

class CRasterFont {
public:
  friend class CFontInstruction;
  CRasterFont(CInputStream& in, IObjectStore* store);

  EFontMode GetMode() const;

  int GetMonoWidth() const;
  int GetMonoHeight() const;
  int GetCarriageAdvance();

  const CGlyph* GetGlyph(wchar_t c) const;
  bool HasGlyph(wchar_t c) const { return GetGlyph(c) != nullptr; }

  void GetSize(const CDrawStringOptions&, int&, int&, const wchar_t*, int) const;
  void SetTexture(TToken< CTexture > token);
  inline TToken< CTexture > GetTexture() { return *mTexture; }

  void DrawString(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                  CTextRenderBuffer* buffer, const wchar_t* str, int length) const;
  void DrawSpace(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                 int length) const;

  void SinglePassDrawString(const CDrawStringOptions& options, int x, int y, int& xOut, int& yOut,
                            CTextRenderBuffer* buffer, const wchar_t* str, int length) const;

  void SetupRenderState();

  int GetBaseLine() const;
  int GetLineMargin();
  bool IsFinishedLoading();

private:
  bool mInitialized;
  int mMonoWidth;
  int mMonoHeight;
  rstl::vector< rstl::pair< wchar_t, CGlyph > > mGlyphs;
  rstl::vector< CKernPair > mKerning;
  EFontMode mMode;
  rstl::optional_object< CFontInfo > mFontInfo;
  rstl::optional_object< TToken< CTexture > > mTexture;
  int mBaseline;
  int mLineMargin;

  static int KernLookup(const rstl::vector< CKernPair >& kerning, int a, const int b);
  const CGlyph* InternalGetGlyph(wchar_t c) const;
};
CHECK_SIZEOF(CRasterFont, 0x94)

#endif // _CRASTERFONT
