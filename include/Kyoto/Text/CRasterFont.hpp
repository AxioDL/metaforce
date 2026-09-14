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
  : x0_(a), x1_(b), x4_(c), x8_fontSize(fontSize) {
    strcpy(xc_name, name);
  }

private:
  bool x0_;
  bool x1_;
  int x4_;
  int x8_fontSize;
  char xc_name[64];
};

CHECK_SIZEOF(CFontInfo, 0x4c)

class CKernPair {
public:
  CKernPair(const wchar_t first, const wchar_t second, const int howMuch) : x4_howMuch(howMuch) {
    x2_second = second;
    x0_first = first;
  }

  wchar_t GetFirst() const { return x0_first; }
  wchar_t GetSecond() const { return x2_second; }
  int GetHowMuch() const { return x4_howMuch; }

private:
  wchar_t x0_first;
  wchar_t x2_second;
  int x4_howMuch;
};
// TODO: breaks clangd
// CHECK_SIZEOF(CKernPair, 0x8)

class CGlyph {
public:
  CGlyph(const int a, const int b, const int c, const float startU, const float startV,
         const float endU, const float endV, const int cellWidth, const int cellHeight,
         const int baseline, const int kernStart)
  : x0_a(a)
  , x2_b(b)
  , x4_c(c)
  , x8_startU(startU)
  , xc_startV(startV)
  , x10_endU(endU)
  , x14_endV(endV)
  , x18_cellWidth(cellWidth)
  , x1a_cellHeight(cellHeight)
  , x1c_baseline(baseline)
  , x1e_kernStart(kernStart) {}

  short GetA() const { return x0_a; }
  short GetB() const { return x2_b; }
  short GetC() const { return x4_c; }
  float GetStartU() const { return x8_startU; }
  float GetStartV() const { return xc_startV; }
  float GetEndU() const { return x10_endU; }
  float GetEndV() const { return x14_endV; }
  short GetCellWidth() const { return x18_cellWidth; }
  short GetCellHeight() const { return x1a_cellHeight; }
  short GetBaseLine() const { return x1c_baseline; }
  int GetKernStart() const { return x1e_kernStart; }

private:
  short x0_a;
  short x2_b;
  short x4_c;
  float x8_startU;
  float xc_startV;
  float x10_endU;
  float x14_endV;
  short x18_cellWidth;
  short x1a_cellHeight;
  short x1c_baseline;
  short x1e_kernStart;
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
  inline TToken< CTexture > GetTexture() { return *x80_texture; }

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
  bool x0_initialized;
  int x4_monoWidth;
  int x8_monoHeight;
  rstl::vector< rstl::pair< wchar_t, CGlyph > > xc_glyphs;
  rstl::vector< CKernPair > x1c_kerning;
  EFontMode x2c_mode;
  rstl::optional_object< CFontInfo > x30_fontInfo;
  rstl::optional_object< TToken< CTexture > > x80_texture;
  int x8c_baseline;
  int x90_lineMargin;

  static int KernLookup(const rstl::vector< CKernPair >& kerning, int a, const int b);
  const CGlyph* InternalGetGlyph(wchar_t c) const;
};
CHECK_SIZEOF(CRasterFont, 0x94)

#endif // _CRASTERFONT
