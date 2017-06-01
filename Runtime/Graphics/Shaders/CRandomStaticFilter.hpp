#ifndef __URDE_CRANDOMSTATICFILTER_HPP__
#define __URDE_CRANDOMSTATICFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CRandomStaticFilter
{
    friend struct CRandomStaticFilterGLDataBindingFactory;
    friend struct CRandomStaticFilterVulkanDataBindingFactory;
    friend struct CRandomStaticFilterMetalDataBindingFactory;
    friend struct CRandomStaticFilterD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CColor color;
        float randOff;
        float discardThres;
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;
    bool m_cookieCutter;

public:
    CRandomStaticFilter(EFilterType type, bool cookieCutter=false);
    CRandomStaticFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CRandomStaticFilter(type) {}
    void draw(const zeus::CColor& color, float t);
    void DrawFilter(EFilterShape, const zeus::CColor& color, float t) { draw(color, t); }

    using _CLS = CRandomStaticFilter;
#include "TMultiBlendShaderDecl.hpp"
};

class CCookieCutterDepthRandomStaticFilter : public CRandomStaticFilter
{
public:
    CCookieCutterDepthRandomStaticFilter(EFilterType type)
    : CRandomStaticFilter(type, true) {}
    CCookieCutterDepthRandomStaticFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CCookieCutterDepthRandomStaticFilter(type) {}
};

}

#endif // __URDE_CRANDOMSTATICFILTER_HPP__
