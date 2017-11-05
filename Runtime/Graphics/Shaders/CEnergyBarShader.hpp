#ifndef __URDE_CENERGYBARSHADER_HPP__
#define __URDE_CENERGYBARSHADER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CEnergyBarShader
{
    friend struct CEnergyBarShaderGLDataBindingFactory;
    friend struct CEnergyBarShaderVulkanDataBindingFactory;
    friend struct CEnergyBarShaderMetalDataBindingFactory;
    friend struct CEnergyBarShaderD3DDataBindingFactory;

public:
    struct Vertex
    {
        zeus::CVector3f pos;
        zeus::CVector2f uv;
    };

private:
    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CColor m_color;
    };
    boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf[3];
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind[3];
    Uniform m_uniform;
    const CTexture* m_tex = nullptr;
    size_t m_maxVerts = 0;

public:
    void updateModelMatrix();
    void draw(const zeus::CColor& color0, const std::vector<Vertex>& verts0,
              const zeus::CColor& color1, const std::vector<Vertex>& verts1,
              const zeus::CColor& color2, const std::vector<Vertex>& verts2,
              const CTexture* tex);

    using _CLS = CEnergyBarShader;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CENERGYBARSHADER_HPP__
