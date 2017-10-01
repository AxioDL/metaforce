#ifndef URDE_CWORLDSHADOWSHADER_HPP
#define URDE_CWORLDSHADOWSHADER_HPP

#include "TShader.hpp"
#include "CColoredQuadFilter.hpp"
#include "CTexturedQuadFilter.hpp"

namespace urde
{

class CWorldShadowShader
{
    friend class CWorldShadowShaderGLDataBindingFactory;
    friend class CWorldShadowShaderVulkanDataBindingFactory;
    friend class CWorldShadowShaderD3DDataBindingFactory;
    friend class CWorldShadowShaderMetalDataBindingFactory;

    boo::ITextureR* m_tex;
    std::experimental::optional<CTexturedQuadFilter> m_prevQuad;
    u32 m_w, m_h;

    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CColor m_color;
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferD* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    boo::IShaderDataBinding* m_zDataBind = nullptr;
    Uniform m_uniform;

    void _buildTex(boo::IGraphicsDataFactory::Context& ctx);

public:
    CWorldShadowShader(u32 w, u32 h);
    void bindRenderTarget();
    void drawBase(float extent);
    void lightenShadow();
    void blendPreviousShadow();
    void resolveTexture();

    u32 GetWidth() const { return m_w; }
    u32 GetHeight() const { return m_h; }

    boo::ITexture* GetTexture() const { return m_tex; }

    using _CLS = CWorldShadowShader;
#include "TShaderDecl.hpp"
};

}

#endif // URDE_CWORLDSHADOWSHADER_HPP
