#pragma once

#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CColoredQuadFilter
{
    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CColor m_color;
    };
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    Uniform m_uniform;

public:
    static void Initialize();
    static void Shutdown();
    static const zeus::CRectangle DefaultRect;
    CColoredQuadFilter(EFilterType type);
    CColoredQuadFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CColoredQuadFilter(type) {}
    void draw(const zeus::CColor& color, const zeus::CRectangle& rect=DefaultRect);
    void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t) { draw(color); }
};

class CWideScreenFilter
{
    CColoredQuadFilter m_top;
    CColoredQuadFilter m_bottom;
public:
    CWideScreenFilter(EFilterType type)
    : m_top(type), m_bottom(type) {}
    CWideScreenFilter(EFilterType type, const TLockedToken<CTexture>&)
    : CWideScreenFilter(type) {}
    void draw(const zeus::CColor& color, float t);
    void DrawFilter(EFilterShape shape, const zeus::CColor& color, float t);

    static float SetViewportToMatch(float t);
    static void SetViewportToFull();
};

}

