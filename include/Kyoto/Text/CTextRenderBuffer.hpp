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
    : x0_color(color), x4_cmd(cmd), x8_x(x), xa_y(y), xc_char(chr), xe_index(index) {}

    uint x0_color;
    ECmd x4_cmd;
    short x8_x;
    short xa_y;
    short xc_char;
    schar xe_index;
  };

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  struct SFontPalette {
    int x0_;
    uint x4_;
    uint x8_;
    rstl::auto_ptr< CGraphicsPalette > xc_palette;
    rstl::auto_ptr< CGraphicsPalette > x14_palette;
    rstl::auto_ptr< CGraphicsPalette > x1c_palette;
    rstl::auto_ptr< CGraphicsPalette > x24_palette;
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
  int GetNumPrimitives() const { return x24_primOffsets.size(); }
  Primitive GetPrimitive(int index) const;
  void SetPrimitive(const Primitive& prim, int index);
  rstl::pair< CVector2i, CVector2i > AccumulateTextBounds();
  bool HasSpaceAvailable(const CVector2i& origin, const CVector2i& extent);

private:
  void VerifyBuffer();

  EMode x0_mode;
  rstl::vector< TToken< CRasterFont > > x4_fonts;
  rstl::vector< CFontImageDef > x14_images;
  rstl::vector< int > x24_primOffsets;
  rstl::vector< signed char > x34_bytecode;
  uint x44_blobSize;
  uint x48_curBytecodeOffset;
  mutable char x4c_activeFont;
  mutable char x4d_activePalette;
  mutable char x4e_queuedFont;
  mutable char x4f_queuedPalette;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  mutable rstl::reserved_vector< SFontPalette, 64 > x50_palettes;
#else
  mutable rstl::reserved_vector< rstl::auto_ptr< CGraphicsPalette >, 64 > x50_palettes;
#endif
  mutable int x254_nextPalette;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CVector2i xb58_;
  CVector2i xb60_;
  bool xb68_;
#endif
};

CHECK_SIZEOF(CTextRenderBuffer, (VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02 ? 0xb6c : 0x258))
NESTED_CHECK_SIZEOF(CTextRenderBuffer, Primitive, 0x10)
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
NESTED_CHECK_SIZEOF(CTextRenderBuffer, SFontPalette, 0x2c)
#endif

#endif // _CTEXTRENDERBUFFER
