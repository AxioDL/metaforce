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

    struct Vert
    {
        zeus::CVector2f m_pos;
        zeus::CVector2f m_uv;
    };
    
    struct Uniform
    {
        zeus::CVector4f m_uv[6];
        float m_opacity = 1.f;
    };
    boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;

public:
    CCameraBlurFilter();
    void draw(float amount);

    using _CLS = CCameraBlurFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CCAMERABLURFILTER_HPP__
