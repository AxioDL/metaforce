#ifndef __URDE_CMAPSURFACESHADER_HPP__
#define __URDE_CMAPSURFACESHADER_HPP__

#include "TShader.hpp"

namespace urde
{

class CMapSurfaceShader
{
    friend struct CMapSurfaceShaderGLDataBindingFactory;
    friend struct CMapSurfaceShaderVulkanDataBindingFactory;
    friend struct CMapSurfaceShaderMetalDataBindingFactory;
    friend struct CMapSurfaceShaderD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CMatrix4f mtx;
        zeus::CColor color;
    };

    Uniform m_uniform;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferS* m_ibo;
    boo::IShaderDataBinding* m_dataBind;

public:
    CMapSurfaceShader(boo::IGraphicsDataFactory::Context& ctx, boo::IGraphicsBufferS* vbo,
                      boo::IGraphicsBufferS* ibo);
    void draw(const zeus::CColor& color, u32 start, u32 count);

    using _CLS = CMapSurfaceShader;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CMAPSURFACESHADER_HPP__
