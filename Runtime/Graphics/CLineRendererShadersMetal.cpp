#include "CLineRendererShaders.hpp"
#include "CLineRenderer.hpp"

namespace urde
{

static const char* VS_METAL_TEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn;\n"
"    float4 colorIn;\n"
"    float4 uvIn;\n"
"};\n"
"\n"
"struct LineUniform\n"
"{\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(constant VertData* va [[ buffer(0) ]],\n"
"                        uint vertId [[ vertex_id ]],\n"
"                        constant LineUniform& line [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant VertData& v = va[vertId];\n"
"    vtf.color = v.colorIn * line.moduColor;\n"
"    vtf.uv = v.uvIn.xy;\n"
"    vtf.position = v.posIn;\n"
"    return vtf;\n"
"}\n";

static const char* FS_METAL_TEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"constexpr sampler samp(address::repeat, filter::linear, mip_filter::linear);\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      texture2d<float> tex0 [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex0.sample(samp, vtf.uv);\n"
"}\n";

static const char* VS_METAL_NOTEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn;\n"
"    float4 colorIn;\n"
"};\n"
"\n"
"struct LineUniform\n"
"{\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(constant VertData* va [[ buffer(0) ]],\n"
"                        uint vertId [[ vertex_id ]],\n"
"                        constant LineUniform& line [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant VertData& v = va[vertId];\n"
"    vtf.color = v.colorIn * line.moduColor;\n"
"    vtf.position = v.posIn;\n"
"    return vtf;\n"
"}\n";

static const char* FS_METAL_NOTEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]])\n"
"{\n"
"    return vtf.color;\n"
"}\n";

struct MetalLineDataBindingFactory : CLineRendererShaders::IDataBindingFactory
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
                                                         texCount, textures);
    }
};

CLineRendererShaders::IDataBindingFactory* CLineRendererShaders::Initialize(boo::MetalDataFactory::Context& ctx)
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

    m_texAlpha = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX, m_texVtxFmt,
                                           CGraphics::g_ViewportSamples,
                                           boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                           boo::Primitive::TriStrips, false, true, false);
    m_texAdditive = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX, m_texVtxFmt,
                                              CGraphics::g_ViewportSamples,
                                              boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                              boo::Primitive::TriStrips, false, false, false);
    m_noTexAlpha = ctx.newShaderPipeline(VS_METAL_NOTEX, FS_METAL_NOTEX, m_noTexVtxFmt,
                                             CGraphics::g_ViewportSamples,
                                             boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                             boo::Primitive::TriStrips, false, true, false);
    m_noTexAdditive = ctx.newShaderPipeline(VS_METAL_NOTEX, FS_METAL_NOTEX, m_noTexVtxFmt,
                                                CGraphics::g_ViewportSamples,
                                                boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                boo::Primitive::TriStrips, false, false, false);

    return new struct MetalLineDataBindingFactory;
}

}
