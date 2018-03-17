#include "CDecalShaders.hpp"
#include "Particle/CDecal.hpp"

namespace urde
{

static const char* VS_METAL_TEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn[4];\n"
"    float4 colorIn;\n"
"    float4 uvsIn[4];\n"
"};\n"
"\n"
"struct DecalUniform\n"
"{\n"
"    float4x4 mvp;\n"
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
"vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],\n"
"                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],\n"
"                        constant DecalUniform& decal [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant VertData& v = va[instId];\n"
"    vtf.color = v.colorIn * decal.moduColor;\n"
"    vtf.uv = v.uvsIn[vertId].xy;\n"
"    vtf.position = decal.mvp * v.posIn[vertId];\n"
"    return vtf;\n"
"}\n";

static const char* FS_METAL_TEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler samp [[ sampler(0) ]],\n"
"                      texture2d<float> tex0 [[ texture(0) ]])\n"
"{\n"
"    return vtf.color * tex0.sample(samp, vtf.uv);\n"
"}\n";

static const char* FS_METAL_TEX_REDTOALPHA =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"    float2 uv;\n"
"};\n"
"\n"
"fragment float4 fmain(VertToFrag vtf [[ stage_in ]],\n"
"                      sampler samp [[ sampler(0) ]],\n"
"                      texture2d<float> tex0 [[ texture(0) ]])\n"
"{\n"
"    return float4(vtf.color.rgb, tex0.sample(samp, vtf.uv).r);\n"
"}\n";

static const char* VS_METAL_NOTEX =
"#include <metal_stdlib>\n"
"using namespace metal;\n"
"struct VertData\n"
"{\n"
"    float4 posIn[4];\n"
"    float4 colorIn;\n"
"};\n"
"\n"
"struct DecalUniform\n"
"{\n"
"    float4x4 mvp;\n"
"    float4 moduColor;\n"
"};\n"
"\n"
"struct VertToFrag\n"
"{\n"
"    float4 position [[ position ]];\n"
"    float4 color;\n"
"};\n"
"\n"
"vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],\n"
"                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],\n"
"                        constant DecalUniform& decal [[ buffer(2) ]])\n"
"{\n"
"    VertToFrag vtf;\n"
"    constant VertData& v = va[instId];\n"
"    vtf.color = v.colorIn * decal.moduColor;\n"
"    vtf.position = decal.mvp * v.posIn[vertId];\n"
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

struct MetalDecalDataBindingFactory : TShader<CDecalShaders>::IDataBindingFactory
{
    boo::ObjToken<boo::IShaderDataBinding> BuildShaderDataBinding(boo::IGraphicsDataFactory::Context& ctx,
                                                                  CDecalShaders& shader)
    {
        CQuadDecal& decal = shader.m_decal;
        const SQuadDescr* desc = decal.m_desc;

        CUVElement* texr = desc->x14_TEX.get();
        int texCount = 0;
        boo::ObjToken<boo::ITexture> textures[1];

        if (texr)
        {
            textures[0] = texr->GetValueTexture(0).GetObj()->GetBooTexture();
            texCount = 1;
        }

        if (decal.m_instBuf)
        {
            boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {decal.m_uniformBuf.get()};

            if (shader.m_regPipeline)
                decal.m_normalDataBind = ctx.newShaderDataBinding(shader.m_regPipeline, nullptr, nullptr,
                                                                  decal.m_instBuf.get(), nullptr, 1, uniforms,
                                                                  nullptr, texCount, textures, nullptr, nullptr);
            if (shader.m_redToAlphaPipeline)
                decal.m_redToAlphaDataBind = ctx.newShaderDataBinding(shader.m_redToAlphaPipeline, nullptr, nullptr,
                                                                      decal.m_instBuf.get(), nullptr, 1, uniforms,
                                                                      nullptr, texCount, textures, nullptr, nullptr);
        }

        return {};
    }
};

TShader<CDecalShaders>::IDataBindingFactory* CDecalShaders::Initialize(boo::MetalDataFactory::Context& ctx)
{
    static const boo::VertexElementDescriptor TexFmtTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3}
    };
    m_vtxFormatTex = ctx.newVertexFormat(9, TexFmtTex);

    static const boo::VertexElementDescriptor TexFmtNoTex[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFormatNoTex = ctx.newVertexFormat(5, TexFmtNoTex);

    m_texZTestNoZWrite = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX, nullptr, nullptr, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    m_texAdditiveZTest = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX, nullptr, nullptr, m_vtxFormatTex,
                                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                               boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    m_texRedToAlphaZTest = ctx.newShaderPipeline(VS_METAL_TEX, FS_METAL_TEX_REDTOALPHA, nullptr, nullptr, m_vtxFormatTex,
                                                 boo::BlendFactor::One, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, true, boo::CullMode::None);

    m_noTexZTestNoZWrite = ctx.newShaderPipeline(VS_METAL_NOTEX, FS_METAL_NOTEX, nullptr, nullptr, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    m_noTexAdditiveZTest = ctx.newShaderPipeline(VS_METAL_NOTEX, FS_METAL_NOTEX, nullptr, nullptr, m_vtxFormatNoTex,
                                                 boo::BlendFactor::SrcAlpha, boo::BlendFactor::One,
                                                 boo::Primitive::TriStrips, boo::ZTest::LEqual, false, true, false, boo::CullMode::None);

    return new struct MetalDecalDataBindingFactory;
}

template <>
void CDecalShaders::Shutdown<boo::MetalDataFactory>()
{
    m_texZTestNoZWrite.reset();
    m_texAdditiveZTest.reset();
    m_texRedToAlphaZTest.reset();

    m_noTexZTestNoZWrite.reset();
    m_noTexAdditiveZTest.reset();

    m_vtxFormatTex.reset();
    m_vtxFormatNoTex.reset();
}

}
