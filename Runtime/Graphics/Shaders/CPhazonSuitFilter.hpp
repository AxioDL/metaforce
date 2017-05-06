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

    boo::GraphicsDataToken m_gfxTok;
    boo::IGraphicsBufferD* m_uniBufBlurX;
    boo::IGraphicsBufferD* m_uniBufBlurY;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IGraphicsBufferS* m_blurVbo;
    boo::IGraphicsBufferS* m_vbo;
    const CTexture* m_indTex = nullptr;
    boo::IShaderDataBinding* m_dataBindBlurX = nullptr;
    boo::IShaderDataBinding* m_dataBindBlurY = nullptr;
    boo::IShaderDataBinding* m_dataBind = nullptr;

public:
    void drawBlurPasses(float radius, const CTexture* indTex);
    void draw(const zeus::CColor& color,
              float indScale, float indOffX, float indOffY);

    using _CLS = CPhazonSuitFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CPHAZONSUITFILTER_HPP__
