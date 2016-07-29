#ifndef __URDE_CSPACEWARPFILTER_HPP__
#define __URDE_CSPACEWARPFILTER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"

namespace urde
{

class CSpaceWarpFilter
{
    friend struct CSpaceWarpFilterGLDataBindingFactory;
    friend struct CSpaceWarpFilterVulkanDataBindingFactory;

    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CMatrix4f m_indXf;
        zeus::CVector3f m_strength;
    };
    u8 m_shiftTexture[4][8][4] = {};
    boo::GraphicsDataToken m_token;
    boo::ITexture* m_warpTex;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

    void GenerateWarpRampTex(boo::IGraphicsDataFactory::Context& ctx);

public:
    CSpaceWarpFilter();
    void setStrength(float scale)
    {
        m_uniform.m_strength[0] = scale;
        m_uniform.m_strength[1] = scale;
    }
    void draw(const zeus::CVector2f& pt);

    using _CLS = CSpaceWarpFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CSPACEWARPFILTER_HPP__
