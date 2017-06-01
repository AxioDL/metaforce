#ifndef __URDE_CCOLOREDQUADFILTER_HPP__
#define __URDE_CCOLOREDQUADFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CColoredQuadFilter
{
    friend struct CColoredQuadFilterGLDataBindingFactory;
    friend struct CColoredQuadFilterVulkanDataBindingFactory;
    friend struct CColoredQuadFilterMetalDataBindingFactory;
    friend struct CColoredQuadFilterD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CColor m_color;
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

public:
    static const zeus::CRectangle DefaultRect;
    CColoredQuadFilter(EFilterType type);
    CColoredQuadFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CColoredQuadFilter(type) {}
    void draw(const zeus::CColor& color, const zeus::CRectangle& rect=DefaultRect);
    void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);

    using _CLS = CColoredQuadFilter;
#include "TMultiBlendShaderDecl.hpp"
};

class CWideScreenFilter
{
    CColoredQuadFilter m_top;
    CColoredQuadFilter m_bottom;
public:
    CWideScreenFilter(EFilterType type)
    : m_top(type), m_bottom(type) {}
    CWideScreenFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CWideScreenFilter(type) {}
    void draw(const zeus::CColor& color, float t);
    void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);

    static float SetViewportToMatch(float t);
    static void SetViewportToFull();
};

}

#endif // __URDE_CCOLOREDQUADFILTER_HPP__
