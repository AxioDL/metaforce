#ifndef __URDE_CTHERMALHOTFILTER_HPP__
#define __URDE_CTHERMALHOTFILTER_HPP__

#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{

class CThermalHotFilter
{
    struct Uniform
    {
        zeus::CColor m_colorRegs[3];
    };
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;

public:
    static void Initialize();
    static void Shutdown();
    CThermalHotFilter();
    void setColorA(const zeus::CColor& color) {m_uniform.m_colorRegs[0] = color;}
    void setColorB(const zeus::CColor& color) {m_uniform.m_colorRegs[1] = color;}
    void setColorC(const zeus::CColor& color) {m_uniform.m_colorRegs[2] = color;}
    void draw();
};

}

#endif // __URDE_CTHERMALHOTFILTER_HPP__
