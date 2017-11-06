#ifndef __URDE_CPHAZONSUITFILTER_HPP__
#define __URDE_CPHAZONSUITFILTER_HPP__

#include "TShader.hpp"

namespace urde
{
class CTexture;

class CPhazonSuitFilter
{
    friend struct CPhazonSuitFilterGLDataBindingFactory;
    friend struct CPhazonSuitFilterVulkanDataBindingFactory;
    friend struct CPhazonSuitFilterMetalDataBindingFactory;
    friend struct CPhazonSuitFilterD3DDataBindingFactory;

    boo::ObjToken<boo::IGraphicsBufferD> m_uniBufBlurX;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBufBlurY;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IGraphicsBufferS> m_blurVbo;
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    const CTexture* m_indTex = nullptr;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBindBlurX;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBindBlurY;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;

public:
    void drawBlurPasses(float radius, const CTexture* indTex);
    void draw(const zeus::CColor& color,
              float indScale, float indOffX, float indOffY);

    using _CLS = CPhazonSuitFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CPHAZONSUITFILTER_HPP__
