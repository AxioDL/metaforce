#ifndef __URDE_CXRAYBLURFILTER_HPP__
#define __URDE_CXRAYBLURFILTER_HPP__

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "CToken.hpp"

namespace urde
{
class CTexture;

class CXRayBlurFilter
{
    struct Uniform
    {
        zeus::CMatrix4f m_uv[8];
    };
    TLockedToken<CTexture> m_paletteTex;
    boo::ObjToken<boo::ITexture> m_booTex;
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;

public:
    static void Initialize();
    static void Shutdown();
    CXRayBlurFilter(TLockedToken<CTexture>& tex);
    void draw(float amount);
};

}

#endif // __URDE_CXRAYBLURFILTER_HPP__
