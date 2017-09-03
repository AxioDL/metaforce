#ifndef __URDE_CAABOXSHADER_HPP__
#define __URDE_CAABOXSHADER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{

class CAABoxShader
{
    friend struct CAABoxShaderGLDataBindingFactory;
    friend struct CAABoxShaderVulkanDataBindingFactory;
    friend struct CAABoxShaderMetalDataBindingFactory;
    friend struct CAABoxShaderD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CMatrix4f m_xf;
        zeus::CColor m_color;
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferD* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;
    bool m_zOnly;

public:
    CAABoxShader(bool zOnly);
    void setAABB(const zeus::CAABox& aabb);
    void draw(const zeus::CColor& color);

    using _CLS = CAABoxShader;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CAABOXSHADER_HPP__