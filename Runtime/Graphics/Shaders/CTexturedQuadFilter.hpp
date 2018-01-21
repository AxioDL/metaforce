#ifndef __URDE_CTEXTUREDQUADFILTER_HPP__
#define __URDE_CTEXTUREDQUADFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"
#include "CToken.hpp"

namespace urde
{

class CTexturedQuadFilter
{
    friend struct CTexturedQuadFilterGLDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaGLDataBindingFactory;
    friend struct CTexturedQuadFilterVulkanDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaVulkanDataBindingFactory;
    friend struct CTexturedQuadFilterMetalDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaMetalDataBindingFactory;
    friend struct CTexturedQuadFilterD3DDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaD3DDataBindingFactory;

protected:
    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CColor m_color;
        float m_lod = 0.f;
    };
    TLockedToken<CTexture> m_tex;
    boo::ObjToken<boo::ITexture> m_booTex;
    boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;
    bool m_gequal;
    bool m_flipRect = false;

    CTexturedQuadFilter(const boo::ObjToken<boo::ITexture>& tex);

public:
    struct Vert
    {
        zeus::CVector3f m_pos;
        zeus::CVector2f m_uv;
    };

    static const zeus::CRectangle DefaultRect;
    CTexturedQuadFilter(EFilterType type, TLockedToken<CTexture> tex);
    CTexturedQuadFilter(EFilterType type, const boo::ObjToken<boo::ITexture>& tex, bool gequal = false);
    CTexturedQuadFilter(const CTexturedQuadFilter&) = delete;
    CTexturedQuadFilter& operator=(const CTexturedQuadFilter&) = delete;
    CTexturedQuadFilter(CTexturedQuadFilter&&) = default;
    CTexturedQuadFilter& operator=(CTexturedQuadFilter&&) = default;
    void draw(const zeus::CColor& color, float uvScale, const zeus::CRectangle& rect=DefaultRect, float z=0.f);
    void drawCropped(const zeus::CColor& color, float uvScale);
    void drawVerts(const zeus::CColor& color, const Vert verts[4], float lod=0.f);
    void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);
    const TLockedToken<CTexture>& GetTex() const { return m_tex; }
    const boo::ObjToken<boo::ITexture>& GetBooTex() const { return m_booTex; }

    using _CLS = CTexturedQuadFilter;
#include "TMultiBlendShaderDecl.hpp"
};

class CTexturedQuadFilterAlpha : public CTexturedQuadFilter
{
    friend struct CTexturedQuadFilterAlphaGLDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaVulkanDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaMetalDataBindingFactory;
    friend struct CTexturedQuadFilterAlphaD3DDataBindingFactory;

public:
    CTexturedQuadFilterAlpha(EFilterType type, TLockedToken<CTexture> tex);
    CTexturedQuadFilterAlpha(EFilterType type, const boo::ObjToken<boo::ITexture>& tex);
    using _CLS = CTexturedQuadFilterAlpha;
#include "TMultiBlendShaderDecl.hpp"
};

}

#endif // __URDE_CTEXTUREDQUADFILTER_HPP__
