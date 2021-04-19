#include "Runtime/GuiSys/CTextRenderBuffer.hpp"

#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Graphics/CGraphicsPalette.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CTextSupportShader.hpp"
#include "Runtime/GuiSys/CFontImageDef.hpp"
#include "Runtime/GuiSys/CFontRenderState.hpp"
#include "Runtime/GuiSys/CInstruction.hpp"
#include "Runtime/GuiSys/CRasterFont.hpp"
#include "Runtime/GuiSys/CTextExecuteBuffer.hpp"

namespace metaforce {

struct CTextRenderBuffer::BooPrimitiveMark {
  Command m_cmd;
  u32 m_bindIdx;
  u32 m_instIdx;

  void SetOpacity(CTextRenderBuffer& rb, float opacity) {
    switch (m_cmd) {
    case Command::CharacterRender: {
      BooFontCharacters& fc = rb.m_fontCharacters[m_bindIdx];
      CTextSupportShader::CharacterInstance& inst = fc.m_charData[m_instIdx];
      inst.m_mulColor.w = opacity;
      fc.m_dirty = true;
      break;
    }
    case Command::ImageRender: {
      BooImage& img = rb.m_images[m_bindIdx];
      img.m_imageData.m_color.w = opacity;
      img.m_dirty = true;
      break;
    }
    default:
      break;
    }
  }
};

CTextRenderBuffer::CTextRenderBuffer(CTextRenderBuffer&&) noexcept = default;

CTextRenderBuffer::CTextRenderBuffer(EMode mode, CGuiWidget::EGuiModelDrawFlags df) : x0_mode(mode), m_drawFlags(df) {}

CTextRenderBuffer::~CTextRenderBuffer() = default;

CTextRenderBuffer& CTextRenderBuffer::operator=(CTextRenderBuffer&&) noexcept = default;

void CTextRenderBuffer::CommitResources() {
  if (m_committed) {
    return;
  }
  m_committed = true;

  m_uniBuf = hsh::create_dynamic_uniform_buffer<CTextSupportShader::Uniform>();
  if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw) {
    m_uniBuf2 = hsh::create_dynamic_uniform_buffer<CTextSupportShader::Uniform>();
  }
  for (BooFontCharacters& chs : m_fontCharacters) {
    if (!chs.m_charData.empty()) {
      CTextSupportShader::BuildCharacterShaderBinding(*this, chs, m_drawFlags);
    }
  }
  for (BooImage& img : m_images) {
    CTextSupportShader::BuildImageShaderBinding(*this, img, m_drawFlags);
  }
}

void CTextRenderBuffer::SetMode(EMode mode) {
  if (mode == EMode::BufferFill) {
    m_images.reserve(m_imagesCount);
    for (BooFontCharacters& fc : m_fontCharacters) {
      fc.m_charData.reserve(fc.m_charCount);
    }
  }
  m_activeFontCh = -1;
  x0_mode = mode;
}

void CTextRenderBuffer::SetPrimitiveOpacity(int idx, float opacity) {
  m_primitiveMarks[idx].SetOpacity(*this, opacity);
}

u32 CTextRenderBuffer::GetPrimitiveCount() const { return m_primitiveMarks.size(); }

void CTextRenderBuffer::Render(const zeus::CColor& col, float time) {
  CommitResources();

  const zeus::CMatrix4f mv = CGraphics::g_GXModelView.toMatrix4f();
  const zeus::CMatrix4f proj = CGraphics::GetPerspectiveProjectionMatrix(true);
  const zeus::CMatrix4f mat = proj * mv;

#if !HSH_PROFILE_MODE
  m_uniBuf.load({mat, col});
  if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw) {
    zeus::CColor colPremul = col * col.a();
    colPremul.a() = col.a();
    m_uniBuf2.load({mat, colPremul});
  }

  for (BooFontCharacters& chs : m_fontCharacters) {
    if (!chs.m_charData.empty()) {
      if (chs.m_dirty) {
        chs.m_instBuf.load(chs.m_charData);
        chs.m_dirty = false;
      }
      chs.m_dataBinding.draw_instanced(0, 4, chs.m_charData.size());
      if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw) {
        chs.m_dataBindingOverdraw.draw_instanced(0, 4, chs.m_charData.size());
      }
    }
  }

  for (BooImage& img : m_images) {
    if (img.m_dirty) {
      img.m_instBuf.load(img.m_imageData);
      img.m_dirty = false;
    }
    const int idx = int(img.m_imageDef.x0_fps * time) % img.m_dataBinding.size();
    img.m_dataBinding[idx].draw_instanced(0, 4, 1);
    if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw) {
      img.m_dataBindingOverdraw[idx].draw_instanced(0, 4, 1);
    }
  }
#endif
}

void CTextRenderBuffer::AddImage(const zeus::CVector2i& offset, const CFontImageDef& image) {
  if (x0_mode == EMode::AllocTally) {
    m_primitiveMarks.emplace_back<BooPrimitiveMark>({Command::ImageRender, m_imagesCount++, 0});
  } else {
    m_images.emplace_back(image, offset);
  }
}

void CTextRenderBuffer::AddCharacter(const zeus::CVector2i& offset, char16_t ch, const zeus::CColor& color) {
  if (m_activeFontCh == UINT32_MAX) {
    return;
  }
  BooFontCharacters& chs = m_fontCharacters[m_activeFontCh];
  if (x0_mode == EMode::AllocTally) {
    m_primitiveMarks.emplace_back<BooPrimitiveMark>({Command::CharacterRender, m_activeFontCh, chs.m_charCount++});
  } else {
    const CGlyph* glyph = chs.m_font.GetObj()->GetGlyph(ch);

    CTextSupportShader::CharacterInstance& inst = chs.m_charData.emplace_back();
    inst.SetMetrics(*glyph, offset);
    inst.m_fontColor = m_main * color;
    inst.m_outlineColor = m_outline * color;
    inst.m_mulColor = zeus::skWhite;
  }
}

void CTextRenderBuffer::AddPaletteChange(const zeus::CColor& main, const zeus::CColor& outline) {
  m_main = main;
  m_outline = outline;
}

void CTextRenderBuffer::AddFontChange(const TToken<CRasterFont>& font) {
  for (size_t i = 0; i < m_fontCharacters.size(); ++i) {
    BooFontCharacters& chs = m_fontCharacters[i];
    if (*chs.m_font.GetObjectTag() == *font.GetObjectTag()) {
      m_activeFontCh = i;
      return;
    }
  }

  m_activeFontCh = m_fontCharacters.size();
  m_fontCharacters.emplace_back(font);
}

bool CTextRenderBuffer::HasSpaceAvailable(const zeus::CVector2i& origin, const zeus::CVector2i& extent) const {
  std::pair<zeus::CVector2i, zeus::CVector2i> bounds = AccumulateTextBounds();
  if (bounds.first.x > bounds.second.x)
    return true;

  if (0 < origin.y)
    return false;

  zeus::CVector2i size = bounds.second - bounds.first;
  return size.y <= extent.y;
}

std::pair<zeus::CVector2i, zeus::CVector2i> CTextRenderBuffer::AccumulateTextBounds() const {
  std::pair<zeus::CVector2i, zeus::CVector2i> ret =
      std::make_pair(zeus::CVector2i{INT_MAX, INT_MAX}, zeus::CVector2i{INT_MIN, INT_MIN});

  for (const BooFontCharacters& chars : m_fontCharacters) {
    for (const CTextSupportShader::CharacterInstance& charInst : chars.m_charData) {
      ret.first.x = std::min(ret.first.x, int(charInst.m_pos[0].x));
      ret.first.y = std::min(ret.first.y, int(charInst.m_pos[0].z));
      ret.second.x = std::max(ret.second.x, int(charInst.m_pos[3].x));
      ret.second.y = std::max(ret.second.y, int(charInst.m_pos[3].z));
    }
  }

  for (const BooImage& imgs : m_images) {
    ret.first.x = std::min(ret.first.x, int(imgs.m_imageData.m_pos[0].x));
    ret.first.y = std::min(ret.first.y, int(imgs.m_imageData.m_pos[0].z));
    ret.second.x = std::max(ret.second.x, int(imgs.m_imageData.m_pos[3].x));
    ret.second.y = std::max(ret.second.y, int(imgs.m_imageData.m_pos[3].z));
  }

  return ret;
}

} // namespace metaforce
