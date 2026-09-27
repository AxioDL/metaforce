#ifndef _CTEXTRENDERBUFFER
#define _CTEXTRENDERBUFFER

#include "Kyoto/Graphics/CGraphicsPalette.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "Kyoto/Text/CTextColor.hpp"

#include "rstl/pair.hpp"
#include <rstl/reserved_vector.hpp>
#include <rstl/vector.hpp>
class CColor;
class CRasterFont;
class CTextRenderBuffer {
public:
  enum ECmd {
    kC_CharacterRender,
    kC_ImageRender,
    kC_FontChange,
    kC_PaletteChange,
    kC_Invalid = -1,
  };
  enum EMode {
    kM_AllocTally,
    kM_BufferFill,
  };

  struct Primitive {
    Primitive(ECmd cmd, short x, short y, short chr, uint color, schar index)
    : mColor(color), mCmd(cmd), mX(x), mY(y), mChar(chr), mIndex(index) {}

    uint mColor;
    ECmd mCmd;
    short mX;
    short mY;
    short mChar;
    schar mIndex;
  };

#if VERSION >= VERSION_GM8P_00
  struct SFontPalette {
    int x0_;
    uint x4_;
    uint x8_;
    rstl::auto_ptr< CGraphicsPalette > mPalette0;
    rstl::auto_ptr< CGraphicsPalette > mPalette1;
    rstl::auto_ptr< CGraphicsPalette > mPalette2;
    rstl::auto_ptr< CGraphicsPalette > mPalette3;
  };
#endif

  CTextRenderBuffer(EMode mode);

  CGraphicsPalette* GetNextAvailablePalette() const;
  int GetMatchingPaletteIndex(const CGraphicsPalette& palette) const;
  void AddFontChange(const TToken< CRasterFont >& font);
  void AddPaletteChange(const CGraphicsPalette& palette);
  void AddCharacter(const CVector2i&, short chr, uint color);
  void AddImage(const CVector2i& offset, const CFontImageDef& image);

  void* GetOutStream();
  size_t GetCurLen();
  void SetMode(EMode mode);
  void Render(const CColor& color, float time) const;
  int GetNumPrimitives() const { return mPrimOffsets.size(); }
  Primitive GetPrimitive(int index) const;
  void SetPrimitive(const Primitive& prim, int index);
  rstl::pair< CVector2i, CVector2i > AccumulateTextBounds();
  bool HasSpaceAvailable(const CVector2i& origin, const CVector2i& extent);

private:
  void VerifyBuffer();

  EMode mMode;
  rstl::vector< TToken< CRasterFont > > mFonts;
  rstl::vector< CFontImageDef > mImages;
  rstl::vector< int > mPrimOffsets;
  rstl::vector< signed char > mBytecode;
  uint mBlobSize;
  uint mCurBytecodeOffset;
  mutable char mActiveFont;
  mutable char mActivePalette;
  mutable char mQueuedFont;
  mutable char mQueuedPalette;
#if VERSION >= VERSION_GM8P_00
  mutable rstl::reserved_vector< SFontPalette, 64 > mPalettes;
#else
  mutable rstl::reserved_vector< rstl::auto_ptr< CGraphicsPalette >, 64 > mPalettes;
#endif
  mutable int mNextPalette;
#if VERSION >= VERSION_GM8P_00
  CVector2i xb58_;
  CVector2i xb60_;
  bool xb68_;
#endif
};

CHECK_SIZEOF(CTextRenderBuffer, (VERSION >= VERSION_GM8P_00 ? 0xb6c : 0x258))
NESTED_CHECK_SIZEOF(CTextRenderBuffer, Primitive, 0x10)
#if VERSION >= VERSION_GM8P_00
NESTED_CHECK_SIZEOF(CTextRenderBuffer, SFontPalette, 0x2c)
#endif

#endif // _CTEXTRENDERBUFFER
