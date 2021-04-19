#pragma once

#include <array>

#include "Runtime/GuiSys/CGuiWidget.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CVector2i.hpp"
#include "zeus/CVector3f.hpp"

namespace metaforce {
class CGlyph;
class CFontImageDef;
class CTextRenderBuffer;
struct BooFontCharacters;
struct BooImage;

class CTextSupportShader {
  friend class CTextRenderBuffer;

public:
  struct Uniform {
    hsh::float4x4 m_mvp;
    hsh::float4 m_uniformColor;
  };

  struct CharacterInstance {
    std::array<hsh::float3, 4> m_pos;
    std::array<hsh::float3, 4> m_uv;
    hsh::float4 m_fontColor;
    hsh::float4 m_outlineColor;
    hsh::float4 m_mulColor;
    void SetMetrics(const CGlyph& glyph, const zeus::CVector2i& offset);
  };

  struct ImageInstance {
    std::array<hsh::float3, 4> m_pos;
    std::array<hsh::float2, 4> m_uv;
    hsh::float4 m_color;
    void SetMetrics(const CFontImageDef& imgDef, const zeus::CVector2i& offset);
  };

  static void BuildCharacterShaderBinding(CTextRenderBuffer& buf, BooFontCharacters& chs,
                                          CGuiWidget::EGuiModelDrawFlags flags);
  static void BuildImageShaderBinding(CTextRenderBuffer& buf, BooImage& img, CGuiWidget::EGuiModelDrawFlags flags);
};

} // namespace metaforce
