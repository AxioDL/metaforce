#ifndef __URDE_CCAMERABLURFILTER_HPP__
#define __URDE_CCAMERABLURFILTER_HPP__

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
    boo::ITexture* m_booTex = nullptr;
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;

public:
    CXRayBlurFilter(TLockedToken<CTexture>& tex);
    void draw(float amount);

    using _CLS = CXRayBlurFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CCAMERABLURFILTER_HPP__
