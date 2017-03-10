#include "specter/RootView.hpp"
#include "specter/TextView.hpp"
#include "specter/ViewResources.hpp"
#include "utf8proc.h"

#include <freetype/internal/internal.h>
#include <freetype/internal/ftobjs.h>

namespace specter
{
static logvisor::Module Log("specter::TextView");

static const char* GLSLVS =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"layout(location=0) in vec3 posIn[4];\n"
"layout(location=4) in mat4 mvMtx;\n"
"layout(location=8) in vec3 uvIn[4];\n"
"layout(location=12) in vec4 colorIn;\n"
SPECTER_GLSL_VIEW_VERT_BLOCK
"struct VertToFrag\n"
"{\n"
"    vec3 uv;\n"
"    vec4 color;\n"
"};\n"
"SBINDING(0) out VertToFrag vtf;\n"
"void main()\n"
"{\n"
"    vtf.uv = uvIn[gl_VertexID];\n"
"    vtf.color = colorIn * mulColor;\n"
"    gl_Position = mv * mvMtx * vec4(posIn[gl_VertexID], 1.0);\n"
"    gl_Position = FLIPFROMGL(gl_Position);\n"
"}\n";

static const char* GLSLFSReg =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"TBINDING0 uniform sampler2DArray fontTex;\n"
"struct VertToFrag\n"
"{\n"
"    vec3 uv;\n"
"    vec4 color;\n"
"};\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0) out vec4 colorOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"    colorOut.a *= texture(fontTex, vtf.uv).r;\n"
"}\n";

static const char* GLSLFSSubpixel =
"#version 330\n"
BOO_GLSL_BINDING_HEAD
"TBINDING0 uniform sampler2DArray fontTex;\n"
"struct VertToFrag\n"
"{\n"
"    vec3 uv;\n"
"    vec4 color;\n"
"};\n"
"SBINDING(0) in VertToFrag vtf;\n"
"layout(location=0, index=0) out vec4 colorOut;\n"
"layout(location=0, index=1) out vec4 blendOut;\n"
"void main()\n"
"{\n"
"    colorOut = vtf.color;\n"
"    blendOut = colorOut.a * texture(fontTex, vtf.uv);\n"
"}\n";

static const char* BlockNames[] = {"SpecterViewBlock"};
static const char* TexNames[] = {"fontTex"};

void TextView::Resources::init(boo::GLDataFactory::Context& ctx, FontCache* fcache)
{
    m_fcache = fcache;

    m_regular =
    ctx.newShaderPipeline(GLSLVS, GLSLFSReg, 1, TexNames, 1, BlockNames,
                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                          boo::Primitive::TriStrips, false, false, boo::CullMode::None);

    m_subpixel =
    ctx.newShaderPipeline(GLSLVS, GLSLFSSubpixel, 1, TexNames, 1, BlockNames,
                          boo::BlendFactor::SrcColor1, boo::BlendFactor::InvSrcColor1,
                          boo::Primitive::TriStrips, false, false, boo::CullMode::None);
}

#if _WIN32

void TextView::Resources::init(boo::ID3DDataFactory::Context& ctx, FontCache* fcache)
{
    m_fcache = fcache;

    static const char* VS =
    "struct VertData\n"
    "{\n"
    "    float3 posIn[4] : POSITION;\n"
    "    float4x4 mvMtx : MODELVIEW;\n"
    "    float3 uvIn[4] : UV;\n"
    "    float4 colorIn : COLOR;\n"
    "};\n"
    SPECTER_HLSL_VIEW_VERT_BLOCK
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float3 uv : UV;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "VertToFrag main(in VertData v, in uint vertId : SV_VertexID)\n"
    "{\n"
    "    VertToFrag vtf;\n"
    "    vtf.uv = v.uvIn[vertId];\n"
    "    vtf.color = v.colorIn * mulColor;\n"
    "    vtf.position = mul(mv, mul(v.mvMtx, float4(v.posIn[vertId], 1.0)));\n"
    "    return vtf;\n"
    "}\n";

    static const char* FSReg =
    "Texture2DArray fontTex : register(t0);\n"
    "SamplerState samp : register(s0);\n"
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float3 uv : UV;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "float4 main(in VertToFrag vtf) : SV_Target0\n"
    "{\n"
    "    float4 colorOut = vtf.color;\n"
    "    colorOut.a *= fontTex.Sample(samp, vtf.uv).r;\n"
    "    return colorOut;\n"
    "}\n";

    static const char* FSSubpixel =
    "Texture2DArray fontTex : register(t0);\n"
    "SamplerState samp : register(s0);\n"
    "struct VertToFrag\n"
    "{\n"
    "    float4 position : SV_Position;\n"
    "    float3 uv : UV;\n"
    "    float4 color : COLOR;\n"
    "};\n"
    "struct BlendOut\n"
    "{\n"
    "    float4 colorOut : SV_Target0;\n"
    "    float4 blendOut : SV_Target1;\n"
    "};\n"
    "BlendOut main(in VertToFrag vtf)\n"
    "{\n"
    "    BlendOut ret;\n"
    "    ret.colorOut = vtf.color;\n"
    "    ret.blendOut = ret.colorOut.a * fontTex.Sample(samp, vtf.uv);\n"
    "    return ret;\n"
    "}\n";

    boo::VertexElementDescriptor vdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFmt = ctx.newVertexFormat(13, vdescs);

    m_regular =
    ctx.newShaderPipeline(VS, FSReg, nullptr, nullptr, nullptr, m_vtxFmt,
                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                          boo::Primitive::TriStrips, false, false, boo::CullMode::None);

    m_subpixel =
    ctx.newShaderPipeline(VS, FSSubpixel, nullptr, nullptr, nullptr, m_vtxFmt,
                          boo::BlendFactor::SrcColor1, boo::BlendFactor::InvSrcColor1,
                          boo::Primitive::TriStrips, false, false, boo::CullMode::None);
}

#endif
#if BOO_HAS_METAL

void TextView::Resources::init(boo::MetalDataFactory::Context& ctx, FontCache* fcache)
{
    m_fcache = fcache;

    static const char* VS =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VertData\n"
    "{\n"
    "    float3 posIn[4];\n"
    "    float4x4 mvMtx;\n"
    "    float3 uvIn[4];\n"
    "    float4 colorIn;\n"
    "};\n"
    SPECTER_METAL_VIEW_VERT_BLOCK
    "struct VertToFrag\n"
    "{\n"
    "    float4 position [[ position ]];\n"
    "    float3 uv;\n"
    "    float4 color;\n"
    "};\n"
    "vertex VertToFrag vmain(constant VertData* va [[ buffer(1) ]],\n"
    "                        uint vertId [[ vertex_id ]], uint instId [[ instance_id ]],\n"
    "                        constant SpecterViewBlock& view [[ buffer(2) ]])\n"
    "{\n"
    "    VertToFrag vtf;\n"
    "    constant VertData& v = va[instId];\n"
    "    vtf.uv = v.uvIn[vertId];\n"
    "    vtf.color = v.colorIn * view.mulColor;\n"
    "    vtf.position = view.mv * v.mvMtx * float4(v.posIn[vertId], 1.0);\n"
    "    return vtf;\n"
    "}\n";

    static const char* FSReg =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "constexpr sampler samp(address::repeat);\n"
    "struct VertToFrag\n"
    "{\n"
    "    float4 position [[ position ]];\n"
    "    float3 uv;\n"
    "    float4 color;\n"
    "};\n"
    "fragment float4 fmain(VertToFrag vtf [[ stage_in ]], texture2d_array<float> fontTex [[ texture(0) ]])\n"
    "{\n"
    "    float4 colorOut = vtf.color;\n"
    "    colorOut.a *= fontTex.sample(samp, vtf.uv.xy, vtf.uv.z).r;\n"
    "    return colorOut;\n"
    "}\n";

    boo::VertexElementDescriptor vdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFmt = ctx.newVertexFormat(13, vdescs);

    m_regular =
    ctx.newShaderPipeline(VS, FSReg, m_vtxFmt, 1,
                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                          boo::Primitive::TriStrips, false, false, false);
}

#endif
#if BOO_HAS_VULKAN

void TextView::Resources::init(boo::VulkanDataFactory::Context& ctx, FontCache* fcache)
{
    m_fcache = fcache;

    boo::VertexElementDescriptor vdescs[] =
    {
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
        {nullptr, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
        {nullptr, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
    };
    m_vtxFmt = ctx.newVertexFormat(13, vdescs);

    m_regular =
    ctx.newShaderPipeline(GLSLVS, GLSLFSReg, m_vtxFmt,
                          boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                          boo::Primitive::TriStrips, false, false, boo::CullMode::None);
}

#endif

void TextView::_commitResources(size_t capacity)
{
    auto& res = rootView().viewRes();
    View::_commitResources(res, [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        buildResources(ctx, res);

        if (capacity)
        {
            m_glyphBuf = res.m_textRes.m_glyphPool.allocateBlock(res.m_factory, capacity);

            boo::IShaderPipeline* shader;
            if (m_fontAtlas.subpixel())
                shader = res.m_textRes.m_subpixel;
            else
                shader = res.m_textRes.m_regular;

            auto vBufInfo = m_glyphBuf.getBufferInfo();
            auto uBufInfo = m_viewVertBlockBuf.getBufferInfo();
            boo::IGraphicsBuffer* uBufs[] = {uBufInfo.first};
            size_t uBufOffs[] = {size_t(uBufInfo.second)};
            size_t uBufSizes[] = {sizeof(ViewBlock)};
            boo::ITexture* texs[] = {m_fontAtlas.texture()};

            if (!res.m_textRes.m_vtxFmt)
            {
                boo::VertexElementDescriptor vdescs[] =
                {
                    {vBufInfo.first, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 0},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 1},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 2},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 3},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
                    {vBufInfo.first, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
                };
                m_vtxFmt = ctx.newVertexFormat(13, vdescs, 0, vBufInfo.second);
                m_shaderBinding = ctx.newShaderDataBinding(shader, m_vtxFmt,
                                                           nullptr, vBufInfo.first, nullptr, 1,
                                                           uBufs, nullptr, uBufOffs, uBufSizes,
                                                           1, texs, 0, vBufInfo.second);
            }
            else
            {
                m_shaderBinding = ctx.newShaderDataBinding(shader, res.m_textRes.m_vtxFmt,
                                                           nullptr, vBufInfo.first, nullptr, 1,
                                                           uBufs, nullptr, uBufOffs, uBufSizes,
                                                           1, texs, 0, vBufInfo.second);
            }
        }
        return true;
    });
}

TextView::TextView(ViewResources& res,
                   View& parentView, const FontAtlas& font,
                   Alignment align, size_t capacity)
: View(res, parentView),
  m_capacity(capacity),
  m_fontAtlas(font),
  m_align(align)
{
    if (size_t(hecl::VertexBufferPool<RenderGlyph>::bucketCapacity()) < capacity)
        Log.report(logvisor::Fatal, "bucket overflow [%" PRISize "/%" PRISize "]",
                   capacity, hecl::VertexBufferPool<RenderGlyph>::bucketCapacity());

    _commitResources(0);
}

TextView::TextView(ViewResources& res, View& parentView, FontTag font, Alignment align, size_t capacity)
: TextView(res, parentView, res.m_textRes.m_fcache->lookupAtlas(font), align, capacity) {}

TextView::RenderGlyph::RenderGlyph(int& adv, const FontAtlas::Glyph& glyph, const zeus::CColor& defaultColor)
{
    m_pos[0].assign(adv + glyph.m_leftPadding, glyph.m_verticalOffset + glyph.m_height, 0.f);
    m_pos[1].assign(adv + glyph.m_leftPadding, glyph.m_verticalOffset, 0.f);
    m_pos[2].assign(adv + glyph.m_leftPadding + glyph.m_width, glyph.m_verticalOffset + glyph.m_height, 0.f);
    m_pos[3].assign(adv + glyph.m_leftPadding + glyph.m_width, glyph.m_verticalOffset, 0.f);
    m_uv[0].assign(glyph.m_uv[0], glyph.m_uv[1], glyph.m_layerFloat);
    m_uv[1].assign(glyph.m_uv[0], glyph.m_uv[3], glyph.m_layerFloat);
    m_uv[2].assign(glyph.m_uv[2], glyph.m_uv[1], glyph.m_layerFloat);
    m_uv[3].assign(glyph.m_uv[2], glyph.m_uv[3], glyph.m_layerFloat);
    m_color = defaultColor;
    adv += glyph.m_advance;
}

int TextView::DoKern(FT_Pos val, const FontAtlas& atlas)
{
    if (!val) return 0;
    val = FT_MulFix(val, atlas.FT_Xscale());

    FT_Pos  orig_x = val;

    /* we scale down kerning values for small ppem values */
    /* to avoid that rounding makes them too big.         */
    /* `25' has been determined heuristically.            */
    if (atlas.FT_XPPem() < 25)
        val = FT_MulDiv(orig_x, atlas.FT_XPPem(), 25);

    return FT_PIX_ROUND(val) >> 6;
}

void TextView::typesetGlyphs(const std::string& str, const zeus::CColor& defaultColor)
{
    UTF8Iterator it(str.begin());
    size_t charLen = str.size() ? std::min(it.countTo(str.end()), m_capacity) : 0;
    _commitResources(charLen);

    uint32_t lCh = -1;
    m_glyphs.clear();
    m_glyphs.reserve(charLen);
    m_glyphInfo.clear();
    m_glyphInfo.reserve(charLen);
    int adv = 0;

    if (charLen)
    {
        for (; it.iter() < str.end() ; ++it)
        {
            utf8proc_int32_t ch = *it;
            if (ch == -1)
            {
                Log.report(logvisor::Warning, "invalid UTF-8 char");
                break;
            }
            if (ch == '\n' || ch == '\0')
                break;

            const FontAtlas::Glyph* glyph = m_fontAtlas.lookupGlyph(ch);
            if (!glyph)
                continue;

            if (lCh != -1)
                adv += DoKern(m_fontAtlas.lookupKern(lCh, glyph->m_glyphIdx), m_fontAtlas);
            m_glyphs.emplace_back(adv, *glyph, defaultColor);
            m_glyphInfo.emplace_back(ch, glyph->m_width, glyph->m_height, adv);

            lCh = glyph->m_glyphIdx;

            if (m_glyphs.size() == m_capacity)
                break;
        }
    }

    if (m_align == Alignment::Right)
    {
        int adj = -adv;
        for (RenderGlyph& g : m_glyphs)
        {
            g.m_pos[0][0] += adj;
            g.m_pos[1][0] += adj;
            g.m_pos[2][0] += adj;
            g.m_pos[3][0] += adj;
        }
    }
    else if (m_align == Alignment::Center)
    {
        int adj = -adv / 2;
        for (RenderGlyph& g : m_glyphs)
        {
            g.m_pos[0][0] += adj;
            g.m_pos[1][0] += adj;
            g.m_pos[2][0] += adj;
            g.m_pos[3][0] += adj;
        }
    }

    m_width = adv;
    invalidateGlyphs();
    updateSize();
}

void TextView::typesetGlyphs(const std::wstring& str, const zeus::CColor& defaultColor)
{
    size_t charLen = std::min(str.size(), m_capacity);
    _commitResources(charLen);

    uint32_t lCh = -1;
    m_glyphs.clear();
    m_glyphs.reserve(charLen);
    m_glyphInfo.clear();
    m_glyphInfo.reserve(charLen);
    int adv = 0;

    for (wchar_t ch : str)
    {
        if (ch == L'\n')
            break;

        const FontAtlas::Glyph* glyph = m_fontAtlas.lookupGlyph(ch);
        if (!glyph)
            continue;

        if (lCh != -1)
            adv += DoKern(m_fontAtlas.lookupKern(lCh, glyph->m_glyphIdx), m_fontAtlas);
        m_glyphs.emplace_back(adv, *glyph, defaultColor);
        m_glyphInfo.emplace_back(ch, glyph->m_width, glyph->m_height, adv);

        lCh = glyph->m_glyphIdx;

        if (m_glyphs.size() == m_capacity)
            break;
    }

    if (m_align == Alignment::Right)
    {
        int adj = -adv;
        for (RenderGlyph& g : m_glyphs)
        {
            g.m_pos[0][0] += adj;
            g.m_pos[1][0] += adj;
            g.m_pos[2][0] += adj;
            g.m_pos[3][0] += adj;
        }
    }
    else if (m_align == Alignment::Center)
    {
        int adj = -adv / 2;
        for (RenderGlyph& g : m_glyphs)
        {
            g.m_pos[0][0] += adj;
            g.m_pos[1][0] += adj;
            g.m_pos[2][0] += adj;
            g.m_pos[3][0] += adj;
        }
    }

    m_width = adv;
    invalidateGlyphs();
    updateSize();
}

void TextView::colorGlyphs(const zeus::CColor& newColor)
{
    for (RenderGlyph& glyph : m_glyphs)
        glyph.m_color = newColor;
    invalidateGlyphs();
}

void TextView::colorGlyphsTypeOn(const zeus::CColor& newColor, float startInterval, float fadeTime)
{
}

void TextView::invalidateGlyphs()
{
    if (m_glyphBuf)
    {
        RenderGlyph* out = m_glyphBuf.access();
        size_t i = 0;
        for (RenderGlyph& glyph : m_glyphs)
            out[i++] = glyph;
    }
}

void TextView::think()
{
}

void TextView::resized(const boo::SWindowRect &root, const boo::SWindowRect& sub)
{
    View::resized(root, sub);
}

void TextView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    if (m_glyphs.size())
    {
        gfxQ->setShaderDataBinding(m_shaderBinding);
        gfxQ->drawInstances(0, 4, m_glyphs.size());
    }
}

std::pair<int,int> TextView::queryGlyphDimensions(size_t pos) const
{
    if (pos >= m_glyphInfo.size())
        Log.report(logvisor::Fatal,
                   "TextView::queryGlyphWidth(%" PRISize ") out of bounds: %" PRISize,
                   pos, m_glyphInfo.size());

    return m_glyphInfo[pos].m_dims;
}

size_t TextView::reverseSelectGlyph(int x) const
{
    size_t ret = 0;
    size_t idx = 1;
    int minDelta = abs(x);
    for (const RenderGlyphInfo& info : m_glyphInfo)
    {
        int thisDelta = abs(info.m_adv-x);
        if (thisDelta < minDelta)
        {
            minDelta = thisDelta;
            ret = idx;
        }
        ++idx;
    }
    return ret;
}

int TextView::queryReverseAdvance(size_t idx) const
{
    if (idx > m_glyphInfo.size())
        Log.report(logvisor::Fatal,
                   "TextView::queryReverseGlyph(%" PRISize ") out of inclusive bounds: %" PRISize,
                   idx, m_glyphInfo.size());
    if (!idx) return 0;
    return m_glyphInfo[idx-1].m_adv;
}

std::pair<size_t,size_t> TextView::queryWholeWordRange(size_t idx) const
{
    if (idx > m_glyphInfo.size())
        Log.report(logvisor::Fatal,
                   "TextView::queryWholeWordRange(%" PRISize ") out of inclusive bounds: %" PRISize,
                   idx, m_glyphInfo.size());
    if (m_glyphInfo.empty())
        return {0,0};

    if (idx == m_glyphInfo.size())
        --idx;

    size_t begin = idx;
    while (begin > 0 && !m_glyphInfo[begin-1].m_space)
        --begin;

    size_t end = idx;
    while (end < m_glyphInfo.size() && !m_glyphInfo[end].m_space)
        ++end;

    return {begin, end-begin};
}

}
