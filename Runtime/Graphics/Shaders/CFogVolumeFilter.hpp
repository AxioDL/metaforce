#ifndef __URDE_CFOGVOLUMEFILTER_HPP__
#define __URDE_CFOGVOLUMEFILTER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"

namespace urde
{

class CFogVolumeFilter
{
    friend struct CFogVolumeFilterGLDataBindingFactory;
    friend struct CFogVolumeFilterVulkanDataBindingFactory;
    friend struct CFogVolumeFilterMetalDataBindingFactory;
    friend struct CFogVolumeFilterD3DDataBindingFactory;

    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind1Way;
    boo::IShaderDataBinding* m_dataBind2Way;

public:
    CFogVolumeFilter();
    void draw2WayPass(const zeus::CColor& color);
    void draw1WayPass(const zeus::CColor& color);

    using _CLS = CFogVolumeFilter;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CFOGVOLUMEFILTER_HPP__
