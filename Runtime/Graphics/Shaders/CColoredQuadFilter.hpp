#ifndef __URDE_CCOLOREDQUADFILTER_HPP__
#define __URDE_CCOLOREDQUADFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
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
        zeus::CColor m_color;
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

public:
    CColoredQuadFilter(CCameraFilterPass::EFilterType type);
    void draw(const zeus::CColor& color);

    using _CLS = CColoredQuadFilter;
#include "TMultiBlendShaderDecl.hpp"
};

}

#endif // __URDE_CCOLOREDQUADFILTER_HPP__
