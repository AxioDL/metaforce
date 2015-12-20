#include "Specter/TextView.hpp"
#include "Specter/ViewResources.hpp"
#include "utf8proc.h"

#include <freetype/internal/internal.h>
#include <freetype/internal/ftobjs.h>

namespace Specter
{
static LogVisor::LogModule Log("Specter::TextView");

void TextView::Resources::init(boo::GLDataFactory* factory, FontCache* fcache)
{
    m_fcache = fcache;

    static const char* VS =
    "#version 330\n"
    "layout(location=0) in vec3 posIn[4];\n"
    "layout(location=4) in mat4 mvMtx;\n"
    "layout(location=8) in vec3 uvIn[4];\n"
    "layout(location=12) in vec4 colorIn;\n"
    SPECTER_VIEW_VERT_BLOCK_GLSL
    "struct VertToFrag\n"
    "{\n"
    "    vec3 uv;\n"
    "    vec4 color;\n"
    "};\n"
    "out VertToFrag vtf;\n"
    "void main()\n"
    "{\n"
    "    vtf.uv = uvIn[gl_VertexID];\n"
    "    vtf.color = colorIn * mulColor;\n"
    "    gl_Position = mv * mvMtx * vec4(posIn[gl_VertexID], 1.0);\n"
    "}\n";

    static const char* FSReg =
    "#version 330\n"
    "uniform sampler2DArray fontTex;\n"
    "struct VertToFrag\n"
    "{\n"
    "    vec3 uv;\n"
    "    vec4 color;\n"
    "};\n"
    "in VertToFrag vtf;\n"
    "layout(location=0) out vec4 colorOut;\n"
    "void main()\n"
    "{\n"
    "    colorOut = vtf.color;\n"
    "    colorOut.a *= texture(fontTex, vtf.uv).r;\n"
    "}\n";

    static const char* FSSubpixel =
    "#version 330\n"
    "uniform sampler2DArray fontTex;\n"
    "struct VertToFrag\n"
    "{\n"
    "    vec3 uv;\n"
    "    vec4 color;\n"
    "};\n"
    "in VertToFrag vtf;\n"
    "layout(location=0, index=0) out vec4 colorOut;\n"
    "layout(location=0, index=1) out vec4 blendOut;\n"
    "void main()\n"
    "{\n"
    "    colorOut = vtf.color;\n"
    "    blendOut = colorOut.a * texture(fontTex, vtf.uv);\n"
    "}\n";

    static const char* BlockNames[] = {"SpecterViewBlock"};

    m_regular =
    factory->newShaderPipeline(VS, FSReg, 1, "fontTex", 1, BlockNames,
                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                               false, false, false);

    m_subpixel =
    factory->newShaderPipeline(VS, FSSubpixel, 1, "fontTex", 1, BlockNames,
                               boo::BlendFactor::SrcColor1, boo::BlendFactor::InvSrcColor1,
                               false, false, false);
}

#if _WIN32

void TextView::Resources::init(boo::ID3DDataFactory* factory, FontCache* fcache)
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
    SPECTER_VIEW_VERT_BLOCK_HLSL
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
    m_vtxFmt = factory->newVertexFormat(13, vdescs);

    ComPtr<ID3DBlob> blobVert;
    ComPtr<ID3DBlob> blobFrag;
    ComPtr<ID3DBlob> blobPipe;
    m_regular =
    factory->newShaderPipeline(VS, FSReg, blobVert, blobFrag, blobPipe, m_vtxFmt,
                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                               false, false, false);

    blobFrag.Reset();
    blobPipe.Reset();
    m_subpixel =
    factory->newShaderPipeline(nullptr, FSSubpixel, blobVert, blobFrag, blobPipe, m_vtxFmt,
                               boo::BlendFactor::SrcColor1, boo::BlendFactor::InvSrcColor1,
                               false, false, false);
}
    
#elif BOO_HAS_METAL
    
void TextView::Resources::init(boo::MetalDataFactory* factory, FontCache* fcache)
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
    SPECTER_VIEW_VERT_BLOCK_METAL
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
    m_vtxFmt = factory->newVertexFormat(13, vdescs);
    
    m_regular =
    factory->newShaderPipeline(VS, FSReg, m_vtxFmt, 1,
                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                               false, false, false);
}
    
#endif

TextView::TextView(ViewResources& res, View& parentView, const FontAtlas& font, Alignment align, size_t capacity)
: View(res, parentView),
  m_capacity(capacity),
  m_fontAtlas(font),
  m_align(align)
{
    m_glyphBuf =
    res.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                    sizeof(RenderGlyph), capacity);

    boo::IShaderPipeline* shader;
    if (font.subpixel())
        shader = res.m_textRes.m_subpixel;
    else
        shader = res.m_textRes.m_regular;

    if (!res.m_textRes.m_vtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {m_glyphBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
            {m_glyphBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
            {m_glyphBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
            {m_glyphBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
            {m_glyphBuf, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 0},
            {m_glyphBuf, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 1},
            {m_glyphBuf, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 2},
            {m_glyphBuf, nullptr, boo::VertexSemantic::ModelView | boo::VertexSemantic::Instanced, 3},
            {m_glyphBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
            {m_glyphBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
            {m_glyphBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
            {m_glyphBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
            {m_glyphBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced}
        };
        m_vtxFmt = res.m_factory->newVertexFormat(13, vdescs);
        boo::ITexture* texs[] = {m_fontAtlas.texture()};
        m_shaderBinding = res.m_factory->newShaderDataBinding(shader, m_vtxFmt,
                                                              nullptr, m_glyphBuf, nullptr, 1,
                                                              (boo::IGraphicsBuffer**)&m_viewVertBlockBuf,
                                                              1, texs);
    }
    else
    {
        boo::ITexture* texs[] = {m_fontAtlas.texture()};
        m_shaderBinding = res.m_factory->newShaderDataBinding(shader, res.m_textRes.m_vtxFmt,
                                                              nullptr, m_glyphBuf, nullptr, 1,
                                                              (boo::IGraphicsBuffer**)&m_viewVertBlockBuf,
                                                              1, texs);
    }

    m_glyphs.reserve(capacity);
    commitResources(res);
}

TextView::TextView(ViewResources& res, View& parentView, FontTag font, Alignment align, size_t capacity)
: TextView(res, parentView, res.m_textRes.m_fcache->lookupAtlas(font), align, capacity) {}

TextView::RenderGlyph::RenderGlyph(int& adv, const FontAtlas::Glyph& glyph, const Zeus::CColor& defaultColor)
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

void TextView::typesetGlyphs(const std::string& str, const Zeus::CColor& defaultColor)
{
    size_t rem = str.size();
    const utf8proc_uint8_t* it = reinterpret_cast<const utf8proc_uint8_t*>(str.data());
    uint32_t lCh = -1;
    m_glyphs.clear();
    m_glyphs.reserve(str.size());
    m_glyphDims.clear();
    m_glyphDims.reserve(str.size());
    m_glyphAdvs.clear();
    m_glyphAdvs.reserve(str.size());
    int adv = 0;

    while (rem)
    {
        utf8proc_int32_t ch;
        utf8proc_ssize_t sz = utf8proc_iterate(it, -1, &ch);
        if (sz < 0)
            Log.report(LogVisor::FatalError, "invalid UTF-8 char");
        if (ch == '\n')
            break;

        const FontAtlas::Glyph* glyph = m_fontAtlas.lookupGlyph(ch);
        if (!glyph)
        {
            rem -= sz;
            it += sz;
            continue;
        }

        if (lCh != -1)
            adv += DoKern(m_fontAtlas.lookupKern(lCh, glyph->m_glyphIdx), m_fontAtlas);
        m_glyphs.emplace_back(adv, *glyph, defaultColor);
        m_glyphDims.emplace_back(glyph->m_width, glyph->m_height);
        m_glyphAdvs.push_back(adv);

        lCh = glyph->m_glyphIdx;
        rem -= sz;
        it += sz;

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
    m_valid = false;
    updateSize();
}

void TextView::typesetGlyphs(const std::wstring& str, const Zeus::CColor& defaultColor)
{
    uint32_t lCh = -1;
    m_glyphs.clear();
    m_glyphs.reserve(str.size());
    m_glyphDims.clear();
    m_glyphDims.reserve(str.size());
    m_glyphAdvs.clear();
    m_glyphAdvs.reserve(str.size());
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
        m_glyphDims.emplace_back(glyph->m_width, glyph->m_height);
        m_glyphAdvs.push_back(adv);

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
    m_valid = false;
    updateSize();
}

void TextView::colorGlyphs(const Zeus::CColor& newColor)
{
    for (RenderGlyph& glyph : m_glyphs)
        glyph.m_color = newColor;
    m_valid = false;
}
void TextView::colorGlyphsTypeOn(const Zeus::CColor& newColor, float startInterval, float fadeTime)
{
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
        if (!m_valid)
        {
            m_glyphBuf->load(m_glyphs.data(), m_glyphs.size() * sizeof(RenderGlyph));
            m_valid = true;
        }
        gfxQ->setShaderDataBinding(m_shaderBinding);
        gfxQ->setDrawPrimitive(boo::Primitive::TriStrips);
        gfxQ->drawInstances(0, 4, m_glyphs.size());
    }
}

std::pair<int,int> TextView::queryGlyphDimensions(size_t pos) const
{
    if (pos >= m_glyphDims.size())
        Log.report(LogVisor::FatalError,
                   "TextView::queryGlyphWidth(%" PRISize ") out of bounds: %" PRISize,
                   pos, m_glyphDims.size());

    return m_glyphDims[pos];
}

size_t TextView::reverseSelectGlyph(int x) const
{
    size_t ret = 0;
    size_t idx = 1;
    int minDelta = abs(x);
    for (int adv : m_glyphAdvs)
    {
        int thisDelta = abs(adv-x);
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
    if (idx > m_glyphAdvs.size())
        Log.report(LogVisor::FatalError,
                   "TextView::queryReverseGlyph(%" PRISize ") out of inclusive bounds: %" PRISize,
                   idx, m_glyphAdvs.size());
    if (!idx) return 0;
    return m_glyphAdvs[idx-1];
}

}
