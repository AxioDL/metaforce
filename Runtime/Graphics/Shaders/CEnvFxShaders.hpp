#pragma once

#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{
class CEnvFxManager;
class CEnvFxManagerGrid;

class CEnvFxShaders
{
public:
    struct Instance
    {
        zeus::CVector3f positions[4];
        zeus::CColor color;
        zeus::CVector2f uvs[4];
    };
    struct Uniform
    {
        zeus::CMatrix4f mvp;
        zeus::CMatrix4f envMtx;
        zeus::CColor moduColor;
    };

private:
    static boo::ObjToken<boo::IShaderPipeline> m_snowPipeline;
    static boo::ObjToken<boo::IShaderPipeline> m_underwaterPipeline;

public:
    static void Initialize();
    static void Shutdown();
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                       CEnvFxManager& fxMgr, CEnvFxManagerGrid& grid);
};

}
