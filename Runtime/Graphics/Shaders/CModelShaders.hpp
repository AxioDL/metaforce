#ifndef __URDE_CMODELSHADERS_HPP__
#define __URDE_CMODELSHADERS_HPP__

#include "hecl/Runtime.hpp"
#include "optional.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"

#define URDE_MAX_LIGHTS 16

namespace urde
{

class CModelShaders
{
    friend class CModel;
    hecl::Runtime::ShaderCacheManager m_shaderCache;
    static std::experimental::optional<CModelShaders> g_ModelShaders;
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensions(boo::IGraphicsDataFactory::Platform plat);
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensionsGLSL(boo::IGraphicsDataFactory::Platform plat);
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensionsHLSL(boo::IGraphicsDataFactory::Platform plat);
    static hecl::Runtime::ShaderCacheExtensions GetShaderExtensionsMetal(boo::IGraphicsDataFactory::Platform plat);
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
    };

    static void Initialize(const hecl::Runtime::FileStoreManager& storeMgr,
                           boo::IGraphicsDataFactory* gfxFactory);

    CModelShaders(const hecl::Runtime::FileStoreManager& storeMgr,
                  boo::IGraphicsDataFactory* gfxFactory);

    boo::IShaderPipeline* buildShader(const hecl::Runtime::ShaderTag& tag, const std::string& source,
                                      const std::string& diagName,
                                      boo::IGraphicsDataFactory::Context& ctx)
    {
        return m_shaderCache.buildShader(tag, source, diagName, ctx);
    }

    boo::IShaderPipeline* buildShader(const hecl::Runtime::ShaderTag& tag, const hecl::Frontend::IR& ir,
                                      const std::string& diagName,
                                      boo::IGraphicsDataFactory::Context& ctx)
    {
        return m_shaderCache.buildShader(tag, ir, diagName, ctx);
    }

    std::vector<boo::IShaderPipeline*> buildExtendedShader(const hecl::Runtime::ShaderTag& tag, const std::string& source,
                                                           const std::string& diagName,
                                                           boo::IGraphicsDataFactory::Context& ctx)
    {
        return m_shaderCache.buildExtendedShader(tag, source, diagName, ctx);
    }

    std::vector<boo::IShaderPipeline*> buildExtendedShader(const hecl::Runtime::ShaderTag& tag, const hecl::Frontend::IR& ir,
                                                           const std::string& diagName,
                                                           boo::IGraphicsDataFactory::Context& ctx)
    {
        return m_shaderCache.buildExtendedShader(tag, ir, diagName, ctx);
    }

};

}

#endif // __URDE_CMODELSHADERS_HPP__
