#ifndef __URDE_CTHERMALHOTFILTER_HPP__
#define __URDE_CTHERMALHOTFILTER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

class CThermalHotFilter
{
    friend struct CThermalHotFilterGLDataBindingFactory;
    friend struct CThermalHotFilterVulkanDataBindingFactory;
    friend struct CThermalHotFilterMetalDataBindingFactory;
    friend struct CThermalHotFilterD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CColor m_colorRegs[3];
    };
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;

public:
    CThermalHotFilter();
    void setColorA(const zeus::CColor& color) {m_uniform.m_colorRegs[0] = color;}
    void setColorB(const zeus::CColor& color) {m_uniform.m_colorRegs[1] = color;}
    void setColorC(const zeus::CColor& color) {m_uniform.m_colorRegs[2] = color;}
    void draw();

    using _CLS = CThermalHotFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CTHERMALHOTFILTER_HPP__
