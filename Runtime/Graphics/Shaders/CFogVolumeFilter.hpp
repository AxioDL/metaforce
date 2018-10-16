#pragma once

#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{

class CFogVolumeFilter
{
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind1Way;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind2Way;

public:
    static void Initialize();
    static void Shutdown();
    CFogVolumeFilter();
    void draw2WayPass(const zeus::CColor& color);
    void draw1WayPass(const zeus::CColor& color);
};

}

