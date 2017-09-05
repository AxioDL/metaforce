#ifndef __URDE_CELEMENTGENSHADERS_HPP__
#define __URDE_CELEMENTGENSHADERS_HPP__

#include "TShader.hpp"
#include "Graphics/CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{
class CElementGen;

class CElementGenShaders
{
    friend struct OGLElementDataBindingFactory;
    friend struct VulkanElementDataBindingFactory;
    friend struct D3DElementDataBindingFactory;
    friend struct MetalElementDataBindingFactory;
public:
    enum class EShaderClass
    {
        Tex,
        IndTex,
        NoTex
    };

private:
    static boo::IShaderPipeline* m_texZTestZWrite;
    static boo::IShaderPipeline* m_texNoZTestZWrite;
    static boo::IShaderPipeline* m_texZTestNoZWrite;
    static boo::IShaderPipeline* m_texNoZTestNoZWrite;
    static boo::IShaderPipeline* m_texAdditiveZTest;
    static boo::IShaderPipeline* m_texAdditiveNoZTest;
    static boo::IShaderPipeline* m_texRedToAlphaZTest;
    static boo::IShaderPipeline* m_texRedToAlphaNoZTest;
    static boo::IShaderPipeline* m_texZTestNoZWriteSub;
    static boo::IShaderPipeline* m_texNoZTestNoZWriteSub;
    static boo::IShaderPipeline* m_texRedToAlphaZTestSub;
    static boo::IShaderPipeline* m_texRedToAlphaNoZTestSub;

    static boo::IShaderPipeline* m_indTexZWrite;
    static boo::IShaderPipeline* m_indTexNoZWrite;
    static boo::IShaderPipeline* m_indTexAdditive;

    static boo::IShaderPipeline* m_cindTexZWrite;
    static boo::IShaderPipeline* m_cindTexNoZWrite;
    static boo::IShaderPipeline* m_cindTexAdditive;

    static boo::IShaderPipeline* m_noTexZTestZWrite;
    static boo::IShaderPipeline* m_noTexNoZTestZWrite;
    static boo::IShaderPipeline* m_noTexZTestNoZWrite;
    static boo::IShaderPipeline* m_noTexNoZTestNoZWrite;
    static boo::IShaderPipeline* m_noTexAdditiveZTest;
    static boo::IShaderPipeline* m_noTexAdditiveNoZTest;

    static boo::IVertexFormat* m_vtxFormatTex; /* No OpenGL */
    static boo::IVertexFormat* m_vtxFormatIndTex; /* No OpenGL */
    static boo::IVertexFormat* m_vtxFormatNoTex; /* No OpenGL */

    CElementGen& m_gen;
    boo::IShaderPipeline* m_regPipeline;
    boo::IShaderPipeline* m_regPipelineSub;
    boo::IShaderPipeline* m_redToAlphaPipeline;
    boo::IShaderPipeline* m_redToAlphaPipelineSub;
    boo::IShaderPipeline* m_regPipelinePmus;
    boo::IShaderPipeline* m_redToAlphaPipelinePmus;
    CElementGenShaders(CElementGen& gen,
                       boo::IShaderPipeline* regPipeline,
                       boo::IShaderPipeline* regPipelineSub,
                       boo::IShaderPipeline* redToAlphaPipeline,
                       boo::IShaderPipeline* redToAlphaPipelineSub,
                       boo::IShaderPipeline* regPipelinePmus,
                       boo::IShaderPipeline* redToAlphaPipelinePmus)
    : m_gen(gen), m_regPipeline(regPipeline), m_regPipelineSub(regPipelineSub),
      m_redToAlphaPipeline(redToAlphaPipeline), m_redToAlphaPipelineSub(redToAlphaPipelineSub),
      m_regPipelinePmus(regPipelinePmus), m_redToAlphaPipelinePmus(redToAlphaPipelinePmus) {}

public:
    static EShaderClass GetShaderClass(CElementGen& gen);
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen);

    using _CLS = CElementGenShaders;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CELEMENTGENSHADERS_HPP__
