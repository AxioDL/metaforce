#include "CLineRendererShaders.hpp"
#include "Graphics/CLineRenderer.hpp"
#include "hecl/VertexBufferPool.hpp"

namespace urde
{

static const char* VS_GLSL_TEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 colorIn;\n"
"layout(location=2) in vec4 uvIn;\n"
"\n"
"UBINDING0 uniform LineUniform\n"
"{\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    vtf.uv = uvIn.xy;\n"
"    gl_Position = FLIPFROMGL(posIn);\n"
"}\n";

static const char* FS_GLSL_TEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"    vec2 uv;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"TBINDING0 uniform sampler2D tex;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(tex, vtf.uv);\n"
"}\n";

static const char* VS_GLSL_NOTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec4 posIn;\n"
"layout(location=1) in vec4 colorIn;\n"
"\n"
"UBINDING0 uniform LineUniform\n"
"{\n"
"    vec4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.color = colorIn * moduColor;\n"
"    gl_Position = FLIPFROMGL(posIn);\n"
"}\n";

static const char* FS_GLSL_NOTEX =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"struct VertToFrag\n"
"{\n"
"    vec4 color;\n"
"};\n"
"\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"}\n";

struct OGLLineDataBindingFactory : CLineRendererShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                CLineRenderer& renderer,
                                const boo::ObjToken<boo::IShaderPipeline>& pipeline,
                                const boo::ObjToken<boo::ITexture>& texture)
    {
        boo::ObjToken<boo::IVertexFormat> vtxFmt;
        int texCount = 0;
        boo::ObjToken<boo::ITexture> textures[1];

        std::pair<boo::ObjToken<boo::IGraphicsBufferD>,
            hecl::VertexBufferPool<CLineRenderer::SDrawVertTex>::IndexTp> vbufInfo;
        std::pair<boo::ObjToken<boo::IGraphicsBufferD>,
            hecl::UniformBufferPool<CLineRenderer::SDrawUniform>::IndexTp> ubufInfo =
            renderer.m_uniformBuf.getBufferInfo();
        if (texture)
        {
            vbufInfo = renderer.m_vertBufTex.getBufferInfo();
            textures[0] = texture;
            texCount = 1;
            const boo::VertexElementDescriptor TexFmtTex[] =
            {
                {vbufInfo.first.get(), nullptr, boo::VertexSemantic::Position4},
                {vbufInfo.first.get(), nullptr, boo::VertexSemantic::Color},
                {vbufInfo.first.get(), nullptr, boo::VertexSemantic::UV4}
            };
            vtxFmt = ctx.newVertexFormat(3, TexFmtTex);
        }
        else
        {
            vbufInfo = renderer.m_vertBufNoTex.getBufferInfo();
            const boo::VertexElementDescriptor TexFmtNoTex[] =
            {
                {vbufInfo.first.get(), nullptr, boo::VertexSemantic::Position4},
                {vbufInfo.first.get(), nullptr, boo::VertexSemantic::Color}
            };
            vtxFmt = ctx.newVertexFormat(2, TexFmtNoTex);
        }

        boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {ubufInfo.first.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        size_t ubufOffs[] = {size_t(ubufInfo.second)};
        size_t ubufSizes[] = {sizeof(CLineRenderer::SDrawUniform)};

        renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, vtxFmt, vbufInfo.first.get(),
                                                         nullptr, nullptr, 1, uniforms, stages,
                                                         ubufOffs, ubufSizes, texCount, textures,
                                                         nullptr, nullptr, vbufInfo.second);
    }
};

CLineRendererShaders::IDataBindingFactory* CLineRendererShaders::Initialize(boo::GLDataFactory::Context& ctx)
{
    static const char* UniNames[] = {"LineUniform"};
    static const char* TexNames[] = {"tex"};

    m_texAlpha = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                       boo::Primitive::TriStrips, boo::ZTest::None,
                                       false, true, false, boo::CullMode::None);
    m_texAdditive = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, TexNames, 1, UniNames,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                          boo::Primitive::TriStrips, boo::ZTest::None,
                                          false, true, false, boo::CullMode::None);
    m_noTexAlpha = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                         boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                         boo::Primitive::TriStrips, boo::ZTest::None,
                                         false, true, false, boo::CullMode::None);
    m_noTexAdditive = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 0, nullptr, 1, UniNames,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                            boo::Primitive::TriStrips, boo::ZTest::None,
                                            false, true, false, boo::CullMode::None);

    return new struct OGLLineDataBindingFactory;
}

#if BOO_HAS_VULKAN
struct VulkanLineDataBindingFactory : CLineRendererShaders::IDataBindingFactory
{
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                boo::IShaderPipeline* pipeline, boo::ITexture* texture)
    {
        int texCount = 0;
        boo::ITexture* textures[1];

        std::pair<boo::IGraphicsBufferD*, hecl::VertexBufferPool<CLineRenderer::SDrawVertTex>::IndexTp> vbufInfo;
        std::pair<boo::IGraphicsBufferD*, hecl::UniformBufferPool<CLineRenderer::SDrawUniform>::IndexTp> ubufInfo =
            renderer.m_uniformBuf.getBufferInfo();
        if (texture)
        {
            vbufInfo = renderer.m_vertBufTex.getBufferInfo();
            textures[0] = texture;
            texCount = 1;
        }
        else
        {
            vbufInfo = renderer.m_vertBufNoTex.getBufferInfo();
        }

        boo::IGraphicsBuffer* uniforms[] = {ubufInfo.first};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        size_t ubufOffs[] = {size_t(ubufInfo.second)};
        size_t ubufSizes[] = {sizeof(CLineRenderer::SDrawUniform)};

        renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, nullptr, vbufInfo.first,
                                                         nullptr, nullptr, 1, uniforms,
                                                         stages, ubufOffs, ubufSizes, texCount, textures,
                                                         nullptr, nullptr, vbufInfo.second);
    }
};

CLineRendererShaders::IDataBindingFactory* CLineRendererShaders::Initialize(boo::VulkanDataFactory::Context& ctx)
{
    static const boo::VertexElementDescriptor VtxFmtTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color},
        {nullptr, nullptr, boo::VertexSemantic::UV4}
    };
    m_texVtxFmt = ctx.newVertexFormat(3, VtxFmtTex);

    static const boo::VertexElementDescriptor VtxFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4},
        {nullptr, nullptr, boo::VertexSemantic::Color}
    };
    m_noTexVtxFmt = ctx.newVertexFormat(2, VtxFmtNoTex);

    m_texAlpha = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_texVtxFmt,
                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                       boo::Primitive::TriStrips, boo::ZTest::None,
                                       true, true, false, boo::CullMode::None);
    m_texAdditive = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_texVtxFmt,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                          boo::Primitive::TriStrips, boo::ZTest::None,
                                          false, true, false, boo::CullMode::None);
    m_noTexAlpha = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_noTexVtxFmt,
                                         boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                         boo::Primitive::TriStrips, boo::ZTest::None,
                                         true, true, false, boo::CullMode::None);
    m_noTexAdditive = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_noTexVtxFmt,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                            boo::Primitive::TriStrips, boo::ZTest::None,
                                            false, true, false, boo::CullMode::None);

    return new struct VulkanLineDataBindingFactory;
}
#endif

}
