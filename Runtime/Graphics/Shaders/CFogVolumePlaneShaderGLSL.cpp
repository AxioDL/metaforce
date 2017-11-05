#include "CFogVolumePlaneShader.hpp"
#include "TShader.hpp"

namespace urde
{

static const char* VS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = posIn;\n"
"}\n";

static const char* FS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vec4(1.0);\n"
"}\n";

URDE_DECL_SPECIALIZE_SHADER(CFogVolumePlaneShader)

static boo::ObjToken<boo::IVertexFormat> s_VtxFmt;
static boo::ObjToken<boo::IShaderPipeline> s_Pipelines[4];

struct CFogVolumePlaneShaderGLDataBindingFactory : TShader<CFogVolumePlaneShader>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CFogVolumePlaneShader& filter)
    {
        boo::GLDataFactory::Context& cctx = static_cast<boo::GLDataFactory::Context&>(ctx);

        const boo::VertexElementDescriptor VtxVmt[] =
        {
            {filter.m_vbo.get(), nullptr, boo::VertexSemantic::Position4}
        };
        boo::ObjToken<boo::IVertexFormat> VtxVmtObj = cctx.newVertexFormat(1, VtxVmt);
        for (int i=0 ; i<4 ; ++i)
            filter.m_dataBinds[i] = cctx.newShaderDataBinding(s_Pipelines[i], VtxVmtObj,
                filter.m_vbo.get(), nullptr, nullptr, 0, nullptr,
                nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBinds[0];
    }
};

#if BOO_HAS_VULKAN
struct CFogVolumePlaneShaderVulkanDataBindingFactory : TShader<CFogVolumePlaneShader>::IDataBindingFactory
{
    boo::IShaderDataBinding* BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                    CFogVolumePlaneShader& filter)
    {
        boo::VulkanDataFactory::Context& cctx = static_cast<boo::VulkanDataFactory::Context&>(ctx);

        for (int i=0 ; i<4 ; ++i)
            filter.m_dataBinds[i] = cctx.newShaderDataBinding(s_Pipelines[i], s_VtxFmt,
                filter.m_vbo, nullptr, nullptr, 0, nullptr,
                nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr);
        return filter.m_dataBinds[0];
    }
};
#endif

TShader<CFogVolumePlaneShader>::IDataBindingFactory*
CFogVolumePlaneShader::Initialize(boo::GLDataFactory::Context& ctx)
{
    s_Pipelines[0] = ctx.newShaderPipeline(VS, FS, 0, nullptr, 0, nullptr, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Frontface);
    s_Pipelines[1] = ctx.newShaderPipeline(VS, FS, 0, nullptr, 0, nullptr, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, false, boo::CullMode::Frontface);
    s_Pipelines[2] = ctx.newShaderPipeline(VS, FS, 0, nullptr, 0, nullptr, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Backface);
    s_Pipelines[3] = ctx.newShaderPipeline(VS, FS, 0, nullptr, 0, nullptr, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::Greater, false, false, false, boo::CullMode::Backface);
    return new CFogVolumePlaneShaderGLDataBindingFactory;
}

template <>
void CFogVolumePlaneShader::Shutdown<boo::GLDataFactory>()
{
    s_Pipelines[0].reset();
    s_Pipelines[1].reset();
    s_Pipelines[2].reset();
    s_Pipelines[3].reset();
}

#if BOO_HAS_VULKAN
TShader<CFogVolumePlaneShader>::IDataBindingFactory*
CFogVolumePlaneShader::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    const boo::VertexElementDescriptor VtxVmt[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4}
    };
    s_VtxFmt = ctx.newVertexFormat(1, VtxVmt);
    s_Pipelines[0] = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Frontface);
    s_Pipelines[1] = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::None, false, false, false, boo::CullMode::Frontface);
    s_Pipelines[2] = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::LEqual, true, false, false, boo::CullMode::Backface);
    s_Pipelines[3] = ctx.newShaderPipeline(VS, FS, s_VtxFmt, boo::BlendFactor::Zero,
                                           boo::BlendFactor::Zero, boo::Primitive::TriStrips,
                                           boo::ZTest::Greater, false, false, false, boo::CullMode::Backface);
    return new CFogVolumePlaneShaderVulkanDataBindingFactory;
}
#endif

}
