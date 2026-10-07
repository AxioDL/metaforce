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

  CKernPair(const CKernPair& other)
  : mFirst(other.mFirst)
  , mSecond(other.mSecond)
  , mHowMuch(other.mHowMuch) {}

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
         const int baseline, const int kernStart, const uchar layer = 0)
  : mA(a)
  , mB(b)
  , mC(c)
  , mStartU(startU)
  , mStartV(startV)
  , mEndU(endU)
  , mEndV(endV)
#if VERSION >= VERSION_GM8P_00
  , mLayer(layer)
#endif
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
  int GetCellWidth() const { return mCellWidth; }
  int GetCellHeight() const { return mCellHeight; }
  int GetBaseLine() const { return mBaseline; }
  int GetKernStart() const { return mKernStart; }

private:
  short mA;
  short mB;
  short mC;
  float mStartU;
  float mStartV;
  float mEndU;
  float mEndV;
#if VERSION >= VERSION_GM8P_00
  uchar mLayer;
  uchar mCellWidth;
  uchar mCellHeight;
  uchar mBaseline;
#else
  short mCellWidth;
  short mCellHeight;
  short mBaseline;
#endif
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

#if VERSION >= VERSION_GM8P_00
  int GetMonoWidth() const { return mMonoWidth; }
  int GetMonoHeight() const { return mMonoHeight; }
#else
  int GetMonoWidth() const;
  int GetMonoHeight() const;
#endif
  int GetCarriageAdvance();

#if VERSION >= VERSION_GM8P_00
  const CGlyph* GetGlyph(wchar_t c) const { return InternalGetGlyph(c); }
#else
  const CGlyph* GetGlyph(wchar_t c) const;
#endif
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

#if VERSION >= VERSION_GM8P_00
  int GetBaseLine() const { return mBaseline; }
  int GetLineMargin() { return mLineMargin; }
#else
  int GetBaseLine() const;
  int GetLineMargin();
#endif
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
