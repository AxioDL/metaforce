#pragma once

#include <utility>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/Shaders/CTextSupportShader.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"

#include "hsh/hsh.h"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CVector2i.hpp"

namespace metaforce {
class CGlyph;
class CGraphicsPalette;
class CRasterFont;
class CTextExecuteBuffer;

using CTextColor = zeus::CColor;

struct BooFontCharacters {
  TLockedToken<CRasterFont> m_font;
  hsh::dynamic_owner<hsh::vertex_buffer<CTextSupportShader::CharacterInstance>> m_instBuf;
  hsh::binding m_dataBinding;
  hsh::binding m_dataBindingOverdraw;
  std::vector<CTextSupportShader::CharacterInstance> m_charData;
  u32 m_charCount = 0;
  bool m_dirty = true;

  BooFontCharacters(const CToken& token) : m_font(token) {}
};

struct BooImage {
  CFontImageDef m_imageDef;
  hsh::dynamic_owner<hsh::vertex_buffer<CTextSupportShader::ImageInstance>> m_instBuf;
  std::vector<hsh::binding> m_dataBinding;
  std::vector<hsh::binding> m_dataBindingOverdraw;
  CTextSupportShader::ImageInstance m_imageData;
  bool m_dirty = true;

  BooImage(const CFontImageDef& imgDef, const zeus::CVector2i& offset) : m_imageDef(imgDef) {
    m_imageData.SetMetrics(imgDef, offset);
  }
};

class CTextRenderBuffer {
  friend class CGuiTextSupport;
  friend class CTextSupportShader;

public:
  enum class Command { CharacterRender, ImageRender, FontChange, PaletteChange };
#if 0
    struct Primitive
    {
        CTextColor x0_color1;
        Command x4_command;
        u16 x8_xPos;
        u16 xa_zPos;
        wchar_t xc_glyph;
        u8 xe_imageIndex;
    };
#endif
  enum class EMode { AllocTally, BufferFill };

private:
  EMode x0_mode{};
#if 0
    std::vector<TToken<CRasterFont>> x4_fonts;
    std::vector<CFontImageDef> x14_images;
    std::vector<int> x24_primOffsets;
    std::vector<char> x34_bytecode;
    u32 x44_blobSize = 0;
    u32 x48_curBytecodeOffset = 0;
    u8 x4c_activeFont;
    u32 x50_paletteCount = 0;
    std::array<std::unique_ptr<CGraphicsPalette>, 64> x54_palettes;
    u32 x254_nextPalette = 0;

#else
  /* Boo-specific text-rendering functionality */
  hsh::dynamic_owner<hsh::uniform_buffer<CTextSupportShader::Uniform>> m_uniBuf, m_uniBuf2;
  std::vector<BooFontCharacters> m_fontCharacters;
  std::vector<BooImage> m_images;

  struct BooPrimitiveMark;
  std::vector<BooPrimitiveMark> m_primitiveMarks;
  u32 m_imagesCount = 0;
  u32 m_activeFontCh = UINT32_MAX;

  zeus::CColor m_main;
  zeus::CColor m_outline = zeus::skBlack;

  CGuiWidget::EGuiModelDrawFlags m_drawFlags{};

  bool m_committed = false;
  void CommitResources();
#endif

public:
  CTextRenderBuffer(CTextRenderBuffer&& other) noexcept;
  CTextRenderBuffer(EMode mode, CGuiWidget::EGuiModelDrawFlags df);
  ~CTextRenderBuffer();

  CTextRenderBuffer& operator=(CTextRenderBuffer&& other) noexcept;

#if 0
    void SetPrimitive(const Primitive&, int);
    Primitive GetPrimitive(int) const;
    void GetOutStream();
    void VerifyBuffer();
    int GetMatchingPaletteIndex(const CGraphicsPalette& palette);
    CGraphicsPalette* GetNextAvailablePalette();
    void AddPaletteChange(const CGraphicsPalette& palette);
#else
  void SetPrimitiveOpacity(int idx, float opacity);
  u32 GetPrimitiveCount() const;
#endif
  void SetMode(EMode mode);
  void Render(const zeus::CColor& col, float time);
  void AddImage(const zeus::CVector2i& offset, const CFontImageDef& image);
  void AddCharacter(const zeus::CVector2i& offset, char16_t ch, const zeus::CColor& color);
  void AddPaletteChange(const zeus::CColor& main, const zeus::CColor& outline);
  void AddFontChange(const TToken<CRasterFont>& font);

  bool HasSpaceAvailable(const zeus::CVector2i& origin, const zeus::CVector2i& extent) const;
  std::pair<zeus::CVector2i, zeus::CVector2i> AccumulateTextBounds() const;
};

} // namespace metaforce
