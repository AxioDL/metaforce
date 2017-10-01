#ifndef __URDE_CMODELSHADERS_HPP__
#define __URDE_CMODELSHADERS_HPP__

#include "hecl/Runtime.hpp"
#include "optional.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "Graphics/CGraphics.hpp"

#define URDE_MAX_LIGHTS 8

namespace urde
{

enum EExtendedShader : uint8_t
{
    Flat,
    Lighting,
    Thermal,
    ForcedAlpha,
    ForcedAdditive,
    SolidColor,
    SolidColorAdditive,
    SolidColorFrontfaceCullLEqualAlphaOnly,
    SolidColorFrontfaceCullAlwaysAlphaOnly, // No Z-write or test
    SolidColorBackfaceCullLEqualAlphaOnly,
    SolidColorBackfaceCullGreaterAlphaOnly, // No Z-write
    MorphBallShadow,
    WorldShadow
};

class CModelShaders
{
    friend class CModel;
    hecl::Runtime::ShaderCacheManager m_shaderCache;
    static std::experimental::optional<CModelShaders> g_ModelShaders;
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensions(boo::IGraphicsDataFactory::Platform plat);
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensionsGLSL(boo::IGraphicsDataFactory::Platform plat);
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensionsHLSL(boo::IGraphicsDataFactory::Platform plat);
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensionsMetal(boo::IGraphicsDataFactory::Platform plat);
    static const hecl::Backend::TextureInfo ThermalTextures[];
    static const hecl::Backend::TextureInfo BallFadeTextures[];
    static const hecl::Backend::TextureInfo WorldShadowTextures[];
public:
    struct Light
    {
        zeus::CVector3f pos;
        zeus::CVector3f dir;
        zeus::CColor color = zeus::CColor::skClear;
        float linAtt[4] = {1.f, 0.f, 0.f};
        float angAtt[4] = {1.f, 0.f, 0.f};
    };

    struct LightingUniform
    {
        Light lights[URDE_MAX_LIGHTS];
        zeus::CColor ambient;
        zeus::CColor colorRegs[3];
        zeus::CColor mulColor;
        CGraphics::CFogState fog;

        void ActivateLights(const std::vector<CLight>& lts);
    };

    struct ThermalUniform
    {
        zeus::CColor mulColor;
        zeus::CColor addColor;
    };

    struct SolidUniform
    {
        zeus::CColor solidColor;
    };

    struct MBShadowUniform
    {
        zeus::CVector4f shadowUp;
        float shadowId;
    };

    static void Initialize(const hecl::Runtime::FileStoreManager& storeMgr,
                           boo::IGraphicsDataFactory* gfxFactory);
    static void Shutdown();

    CModelShaders(const hecl::Runtime::FileStoreManager& storeMgr,
                  boo::IGraphicsDataFactory* gfxFactory);

    std::shared_ptr<hecl::Runtime::ShaderPipelines> buildShader(const hecl::Runtime::ShaderTag& tag,
                                                                const std::string& source,
                                                                const std::string& diagName,
                                                                boo::IGraphicsDataFactory& factory)
    {
        return m_shaderCache.buildShader(tag, source, diagName, factory);
    }

    std::shared_ptr<hecl::Runtime::ShaderPipelines> buildShader(const hecl::Runtime::ShaderTag& tag,
                                                                const hecl::Frontend::IR& ir,
                                                                const std::string& diagName,
                                                                boo::IGraphicsDataFactory& factory)
    {
        return m_shaderCache.buildShader(tag, ir, diagName, factory);
    }

    std::shared_ptr<hecl::Runtime::ShaderPipelines> buildExtendedShader(const hecl::Runtime::ShaderTag& tag,
                                                                        const std::string& source,
                                                                        const std::string& diagName,
                                                                        boo::IGraphicsDataFactory& factory)
    {
        return m_shaderCache.buildExtendedShader(tag, source, diagName, factory);
    }

    std::shared_ptr<hecl::Runtime::ShaderPipelines> buildExtendedShader(const hecl::Runtime::ShaderTag& tag,
                                                                        const hecl::Frontend::IR& ir,
                                                                        const std::string& diagName,
                                                                        boo::IGraphicsDataFactory& factory)
    {
        return m_shaderCache.buildExtendedShader(tag, ir, diagName, factory);
    }

};

}

#endif // __URDE_CMODELSHADERS_HPP__
