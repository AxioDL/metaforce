#include "CLineRendererShaders.hpp"
#include "Graphics/CLineRenderer.hpp"
#if BOO_HAS_METAL

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
    void BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                CLineRenderer& renderer,
                                const boo::ObjToken<boo::IShaderPipeline>& pipeline,
                                const boo::ObjToken<boo::ITexture>& texture)
    {
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
        }
        else
        {
            vbufInfo = renderer.m_vertBufNoTex.getBufferInfo();
        }

        boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {ubufInfo.first.get()};
        boo::PipelineStage stages[] = {boo::PipelineStage::Vertex};
        size_t ubufOffs[] = {ubufInfo.second};
        size_t ubufSizes[] = {sizeof(CLineRenderer::SDrawUniform)};

        renderer.m_shaderBind = ctx.newShaderDataBinding(pipeline, nullptr, vbufInfo.first.get(),
                                                         nullptr, nullptr, 1, uniforms, stages,
                                                         ubufOffs, ubufSizes, texCount, textures,
                                                         nullptr, nullptr, vbufInfo.second);
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

    m_texAlpha = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX, nullptr, nullptr, m_texVtxFmt,
                                       CGraphics::g_ViewportSamples,
                                       boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                       boo::Primitive::TriStrips, boo::ZTest::None,
                                       false, true, false, boo::CullMode::None);
    m_texAdditive = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX, nullptr, nullptr, m_texVtxFmt,
                                          CGraphics::g_ViewportSamples,
                                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                          boo::Primitive::TriStrips, boo::ZTest::None,
                                          false, true, false, boo::CullMode::None);
    m_noTexAlpha = ctx.newShaderPipeline(VS_METAL_NOTEX, FS_METAL_NOTEX, nullptr, nullptr, m_noTexVtxFmt,
                                         CGraphics::g_ViewportSamples,
                                         boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                         boo::Primitive::TriStrips, boo::ZTest::None,
                                         false, true, false, boo::CullMode::None);
    m_noTexAdditive = ctx.newShaderPipeline(VS_METAL_NOTEX, FS_METAL_NOTEX, nullptr, nullptr, m_noTexVtxFmt,
                                            CGraphics::g_ViewportSamples,
                                            boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                            boo::Primitive::TriStrips, boo::ZTest::None,
                                            false, true, false, boo::CullMode::None);

    return new struct MetalLineDataBindingFactory;
}

}
#endif
