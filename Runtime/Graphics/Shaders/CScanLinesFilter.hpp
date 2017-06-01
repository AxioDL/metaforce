#ifndef __URDE_CSCANLINESFILTER_HPP__
#define __URDE_CSCANLINESFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CScanLinesFilter
{
    friend struct CScanLinesFilterGLDataBindingFactory;
    friend struct CScanLinesFilterVulkanDataBindingFactory;
    friend struct CScanLinesFilterMetalDataBindingFactory;
    friend struct CScanLinesFilterD3DDataBindingFactory;

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
    CScanLinesFilter(EFilterType type);
    CScanLinesFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CScanLinesFilter(type) {}
    void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);

    using _CLS = CScanLinesFilter;
#include "TMultiBlendShaderDecl.hpp"
};

}

#endif // __URDE_CSCANLINESFILTER_HPP__
