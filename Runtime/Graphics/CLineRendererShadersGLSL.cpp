#include "CLineRendererShaders.hpp"
#include "CLineRenderer.hpp"

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
"TBINDING0 uniform sampler2D texs[1];\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color * texture(texs[0], vtf.uv);\n"
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
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx, CLineRenderer& renderer,
                                boo::IShaderPipeline* pipeline, boo::ITexture* texture)
    {
        boo::IVertexFormat* vtxFmt = nullptr;
        int texCount = 0;
        boo::ITexture* textures[1];

        if (texture)
        {
            textures[0] = texture;
            texCount = 1;
            const boo::VertexElementDescriptor TexFmtTex[] =
            {
                {renderer.m_vertBuf, nullptr, boo::VertexSemantic::Position4},
                {renderer.m_vertBuf, nullptr, boo::VertexSemantic::Color},
                {renderer.m_vertBuf, nullptr, boo::VertexSemantic::UV4}
            };
            vtxFmt = ctx.newVertexFormat(3, TexFmtTex);
        }
        else
        {
            const boo::VertexElementDescriptor TexFmtNoTex[] =
            {
                {renderer.m_vertBuf, nullptr, boo::VertexSemantic::Position4},
                {renderer.m_vertBuf, nullptr, boo::VertexSemantic::Color}
            };
            vtxFmt = ctx.newVertexFormat(2, TexFmtNoTex);
        }

        boo::IGraphicsBuffer* uniforms[] = {renderer.m_uniformBuf};

        renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, vtxFmt, renderer.m_vertBuf,
                                                         nullptr, nullptr, 1, uniforms, nullptr,
                                                         texCount, textures);
    }
};

CLineRendererShaders::IDataBindingFactory* CLineRendererShaders::Initialize(boo::GLDataFactory::Context& ctx)
{
    static const char* UniNames[] = {"LineUniform"};

    m_texAlpha = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                           boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                           boo::Primitive::TriStrips, false, true, false);
    m_texAdditive = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, 1, "texs", 1, UniNames,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              boo::Primitive::TriStrips, false, false, false);
    m_noTexAlpha = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 1, nullptr, 1, UniNames,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, false, true, false);
    m_noTexAdditive = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, 1, nullptr, 1, UniNames,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                boo::Primitive::TriStrips, false, false, false);

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

        if (texture)
        {
            textures[0] = texture;
            texCount = 1;
        }

        boo::IGraphicsBuffer* uniforms[] = {renderer.m_uniformBuf};

        renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, nullptr, renderer.m_vertBuf,
                                                         nullptr, nullptr, 1, uniforms,
                                                         nullptr, texCount, textures);
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
                                           boo::Primitive::TriStrips, false, true, false);
    m_texAdditive = ctx.newShaderPipeline(VS_GLSL_TEX, FS_GLSL_TEX, m_texVtxFmt,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              boo::Primitive::TriStrips, false, false, false);
    m_noTexAlpha = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_noTexVtxFmt,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, false, true, false);
    m_noTexAdditive = ctx.newShaderPipeline(VS_GLSL_NOTEX, FS_GLSL_NOTEX, m_noTexVtxFmt,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                boo::Primitive::TriStrips, false, false, false);

    return new struct VulkanLineDataBindingFactory;
}
#endif

}
