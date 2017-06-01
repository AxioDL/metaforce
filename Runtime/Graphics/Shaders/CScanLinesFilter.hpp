#ifndef __URDE_CSCANLINESFILTER_HPP__
#define __URDE_CSCANLINESFILTER_HPP__

#include "TMultiBlendShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CScanLinesFilter
{
    friend struct CScanLinesFilterGLDataBindingFactory;
    friend struct CScanLinesFilterVulkanDataBindingFactory;
    friend struct CScanLinesFilterMetalDataBindingFactory;
    friend struct CScanLinesFilterD3DDataBindingFactory;

    struct Uniform
    {
        zeus::CColor color;
    };
    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind = nullptr;
    Uniform m_uniform;
    bool m_even;

public:
    CScanLinesFilter(EFilterType type, bool even);
    void draw(const zeus::CColor& color);
    void DrawFilter(EFilterShape, const zeus::CColor& color, float) { draw(color); }

    using _CLS = CScanLinesFilter;
#include "TMultiBlendShaderDecl.hpp"
};

class CScanLinesFilterEven : public CScanLinesFilter
{
public:
    CScanLinesFilterEven(EFilterType type)
    : CScanLinesFilter(type, true) {}
    CScanLinesFilterEven(EFilterType type, const TLockedToken<CTexture>&)
    : CScanLinesFilterEven(type) {}
};

class CScanLinesFilterOdd : public CScanLinesFilter
{
public:
    CScanLinesFilterOdd(EFilterType type)
    : CScanLinesFilter(type, false) {}
    CScanLinesFilterOdd(EFilterType type, const TLockedToken<CTexture>&)
    : CScanLinesFilterOdd(type) {}
};

}

#endif // __URDE_CSCANLINESFILTER_HPP__
