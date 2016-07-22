#ifndef __URDE_CTHERMALCOLDFILTER_HPP__
#define __URDE_CTHERMALCOLDFILTER_HPP__

#include "TFilterShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

class CThermalColdFilter
{
    friend struct CThermalColdFilterGLDataBindingFactory;
    friend struct CThermalColdFilterVulkanDataBindingFactory;

    struct Uniform
    {
        zeus::CMatrix4f m_shiftTexMtx;
        zeus::CMatrix4f m_indMtx;
        zeus::CColor m_colorRegs[3];
    };
    u8 m_shiftTexture[4][8][4] = {};
    boo::GraphicsDataToken m_token;
    boo::ITextureD* m_shiftTex = nullptr;
    boo::IGraphicsBufferD* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

public:
    CThermalColdFilter();
    void setShift(unsigned shift);
    void setColorA(const zeus::CColor& color) {m_uniform.m_colorRegs[0] = color;}
    void setColorB(const zeus::CColor& color) {m_uniform.m_colorRegs[1] = color;}
    void setColorC(const zeus::CColor& color) {m_uniform.m_colorRegs[2] = color;}
    void setScale(float scale)
    {
        scale = 0.1f * (1.f - scale);
        m_uniform.m_indMtx[0][0] = scale;
        m_uniform.m_indMtx[1][1] = scale;
    }
    void draw();

    using _CLS = CThermalColdFilter;
#include "TFilterDecl.hpp"
};

}

#endif // __URDE_CTHERMALCOLDFILTER_HPP__
