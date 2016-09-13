#ifndef __URDE_CTEXTUREDQUADFILTER_HPP__
#define __URDE_CTEXTUREDQUADFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "Camera/CCameraFilter.hpp"
#include "CToken.hpp"

namespace urde
{

class CTexturedQuadFilter
{
    friend struct CTexturedQuadFilterGLDataBindingFactory;
    friend struct CTexturedQuadFilterVulkanDataBindingFactory;
    friend struct CTexturedQuadFilterMetalDataBindingFactory;
    friend struct CTexturedQuadFilterD3DDataBindingFactory;

    struct Vert
    {
        zeus::CVector2f m_pos;
        zeus::CVector2f m_uv;
    };

    struct Uniform
    {
        zeus::CColor m_color;
        float m_uvScale;
    };
    TLockedToken<CTexture> m_tex;
    boo::ITexture* m_booTex;
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferD* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

public:
    CTexturedQuadFilter(CCameraFilterPass::EFilterType type, TLockedToken<CTexture> tex);
    CTexturedQuadFilter(CCameraFilterPass::EFilterType type, boo::ITexture* tex);
    void draw(const zeus::CColor& color, float uvScale);
    const TLockedToken<CTexture>& GetTex() const { return m_tex; }

    using _CLS = CTexturedQuadFilter;
#include "TMultiBlendShaderDecl.hpp"
};

}

#endif // __URDE_CTEXTUREDQUADFILTER_HPP__
