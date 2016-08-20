#ifndef __URDE_CCAMERABLURFILTER_HPP__
#define __URDE_CCAMERABLURFILTER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

class CCameraBlurFilter
{
    friend struct CCameraBlurFilterGLDataBindingFactory;
    friend struct CCameraBlurFilterVulkanDataBindingFactory;
    friend struct CCameraBlurFilterMetalDataBindingFactory;
    friend struct CCameraBlurFilterD3DDataBindingFactory;

    struct Uniform
    {
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

public:
    CCameraBlurFilter();
    void draw();

    using _CLS = CCameraBlurFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CCAMERABLURFILTER_HPP__
