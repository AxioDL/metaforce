#ifndef __URDE_CELEMENTGENSHADERS_HPP__
#define __URDE_CELEMENTGENSHADERS_HPP__

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
public:
    struct IDataBindingFactory
    {
        virtual void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                            CElementGen& gen,
                                            boo::IShaderPipeline* regPipeline,
                                            boo::IShaderPipeline* redToAlphaPipeline)=0;
    };

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

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;
    static boo::GraphicsDataToken m_gfxToken;

public:
    static IDataBindingFactory* Initialize(boo::GLDataFactory::Context& ctx);
#if _WIN32
    static IDataBindingFactory* Initialize(boo::ID3DDataFactory::Context& ctx);
#endif
#if BOO_HAS_METAL
    static IDataBindingFactory* Initialize(boo::MetalDataFactory::Context& ctx);
#endif
#if BOO_HAS_VULKAN
    static IDataBindingFactory* Initialize(boo::VulkanDataFactory::Context& ctx);
#endif

    static void Initialize();
    static void Shutdown();
    static EShaderClass GetShaderClass(CElementGen& gen);
    static void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CElementGen& gen);
};

}

#endif // __URDE_CELEMENTGENSHADERS_HPP__
