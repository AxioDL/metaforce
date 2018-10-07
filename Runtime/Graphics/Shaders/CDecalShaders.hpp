#ifndef __URDE_CDECALSHADERS_HPP__
#define __URDE_CDECALSHADERS_HPP__

#include "Graphics/CGraphics.hpp"

namespace urde
{
struct CQuadDecal;

class CDecalShaders
{
private:
    static boo::ObjToken<boo::IShaderPipeline> m_texZTestNoZWrite;
    static boo::ObjToken<boo::IShaderPipeline> m_texAdditiveZTest;
    static boo::ObjToken<boo::IShaderPipeline> m_texRedToAlphaZTest;

    static boo::ObjToken<boo::IShaderPipeline> m_noTexZTestNoZWrite;
    static boo::ObjToken<boo::IShaderPipeline> m_noTexAdditiveZTest;

public:
    static void Initialize();
    static void Shutdown();
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CQuadDecal& decal);
};

}

#endif // __URDE_CDECALSHADERS_HPP__
