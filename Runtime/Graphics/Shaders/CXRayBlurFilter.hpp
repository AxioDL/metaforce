#ifndef __URDE_CXRAYBLURFILTER_HPP__
#define __URDE_CXRAYBLURFILTER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "CToken.hpp"

namespace urde
{
class CTexture;

class CXRayBlurFilter
{
    friend struct CXRayBlurFilterGLDataBindingFactory;
    friend struct CXRayBlurFilterVulkanDataBindingFactory;
    friend struct CXRayBlurFilterMetalDataBindingFactory;
    friend struct CXRayBlurFilterD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CMatrix4f m_uv[4];
    };
    TLockedToken<CTexture> m_paletteTex;
    boo::ObjToken<boo::ITexture> m_booTex;
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;

public:
    CXRayBlurFilter(TLockedToken<CTexture>& tex);
    void draw(float amount);

    using _CLS = CXRayBlurFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CXRAYBLURFILTER_HPP__
