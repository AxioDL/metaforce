#pragma once

#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector2i.hpp>

namespace metaforce {
class CGraphicsPalette;
class CTextExecuteBuffer;

using CTextColor = zeus::CColor;

class CTextRenderBuffer {
  friend class CGuiTextSupport;
  friend class CTextSupportShader;

public:
  enum class Command { CharacterRender, ImageRender, FontChange, PaletteChange, Invalid = -1 };
  struct Primitive {
    CTextColor x0_color1;
    Command x4_command;
    s16 x8_xPos;
    s16 xa_zPos;
    char16_t xc_glyph;
    u8 xe_imageIndex;
  };
  enum class EMode { AllocTally, BufferFill };

  struct SFontPalette {
    EFontMode x0_mode;
    std::array<u8, 8> x4_colorPal;
    std::unique_ptr<CGraphicsPalette> xc_layer1Pal;
    std::unique_ptr<CGraphicsPalette> x14_layer2Pal;
    std::unique_ptr<CGraphicsPalette> x1c_layer3Pal;
    std::unique_ptr<CGraphicsPalette> x24_layer4Pal;

    SFontPalette(EFontMode mode, std::unique_ptr<CGraphicsPalette>&& pal1, std::unique_ptr<CGraphicsPalette>&& pal2,
                 std::unique_ptr<CGraphicsPalette>&& pal3, std::unique_ptr<CGraphicsPalette>&& pal4)
    : x0_mode(mode)
    , xc_layer1Pal(std::move(pal1))
    , x14_layer2Pal(std::move(pal2))
    , x1c_layer3Pal(std::move(pal3))
    , x24_layer4Pal(std::move(pal4)) {}
  };

private:
  EMode x0_mode;
  std::vector<TToken<CRasterFont>> x4_fonts;
  std::vector<CFontImageDef> x14_images;
  std::vector<int> x24_primOffsets;
  std::vector<char> x34_bytecode;
  u32 x44_blobSize = 0;
  u32 x48_curBytecodeOffset = 0;
  s8 x4c_activeFont = -1;
  s8 x4d_activePalette = -1;
  s8 x4e_queuedFont = -1;
  s8 x4f_queuedPalette = -1;
  rstl::reserved_vector<SFontPalette, 64> x50_palettes;
  s32 x254_nextPalette = 0;

public:
  CTextRenderBuffer(CTextRenderBuffer&& other) noexcept;
  CTextRenderBuffer(EMode mode);
  ~CTextRenderBuffer();

  CTextRenderBuffer& operator=(CTextRenderBuffer&& other) noexcept;

  void SetPrimitive(const Primitive&, int);
  [[nodiscard]] Primitive GetPrimitive(int) const;
  [[nodiscard]] u32 GetPrimitiveCount() const { return x24_primOffsets.size(); }
  [[nodiscard]] u8* GetOutStream();
  [[nodiscard]] u32 GetCurLen();
  void VerifyBuffer();
  int GetMatchingPaletteIndex(const CGraphicsPalette& palette);
  [[nodiscard]] SFontPalette* GetNextAvailablePalette();
  void AddPaletteChange(const CGraphicsPalette& palette, EFontMode mode);
  void SetMode(EMode mode);
  void Render(const CTextColor& col, float time);
  void AddImage(const zeus::CVector2i& offset, const CFontImageDef& image);
  void AddCharacter(const zeus::CVector2i& offset, char16_t ch, const CTextColor& color);
  void AddFontChange(const TToken<CRasterFont>& font);

  [[nodiscard]] bool HasSpaceAvailable(const zeus::CVector2i& origin, const zeus::CVector2i& extent);
  [[nodiscard]] std::pair<zeus::CVector2i, zeus::CVector2i> AccumulateTextBounds();
};

} // namespace metaforce
