#include "Specter/TextView.hpp"
#include "Specter/ViewSystem.hpp"
#include "utf8proc.h"

#include <freetype/internal/internal.h>
#include <freetype/internal/ftobjs.h>

namespace Specter
{
static LogVisor::LogModule Log("Specter::TextView");

void TextView::System::init(boo::GLDataFactory* factory, FontCache* fcache)
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
    "    vtf.color = colorIn;\n"
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
    "    colorOut.a = texture(fontTex, vtf.uv).r;\n"
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
    "    blendOut = texture(fontTex, vtf.uv);\n"
    "}\n";

    static const char* BlockNames[] = {"SpecterViewBlock"};

    m_regular =
    factory->newShaderPipeline(VS, FSReg, 1, "fontTex", 1, BlockNames,
                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                               true, true, false);

    m_subpixel =
    factory->newShaderPipeline(VS, FSSubpixel, 1, "fontTex", 1, BlockNames,
                               boo::BlendFactor::SrcColor1, boo::BlendFactor::InvSrcColor1,
                               true, true, false);
}

TextView::TextView(ViewSystem& system, FontTag font, size_t capacity)
: View(system),
  m_capacity(capacity),
  m_fontAtlas(system.m_textSystem.m_fcache->lookupAtlas(font))
{
    m_glyphBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(RenderGlyph), capacity);

    if (!system.m_textSystem.m_vtxFmt)
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
        m_vtxFmt = system.m_factory->newVertexFormat(13, vdescs);
        boo::ITexture* texs[] = {m_fontAtlas.texture()};
        m_shaderBinding = system.m_factory->newShaderDataBinding(system.m_textSystem.m_regular, m_vtxFmt,
                                                                 nullptr, m_glyphBuf, nullptr, 1,
                                                                 (boo::IGraphicsBuffer**)&m_viewVertBlockBuf,
                                                                 1, texs);
    }

    m_glyphs.reserve(capacity);
}

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

static int DoKern(FT_Pos val, const FontAtlas& atlas)
{
    val = FT_MulFix(val, atlas.FT_Xscale());

    FT_Pos  orig_x = val;

    /* we scale down kerning values for small ppem values */
    /* to avoid that rounding makes them too big.         */
    /* `25' has been determined heuristically.            */
    if (atlas.FT_XPPem() < 25)
        val = FT_MulDiv(orig_x, atlas.FT_XPPem(), 25);

    return FT_PIX_ROUND(val);
}

void TextView::typesetGlyphs(const std::string& str, const Zeus::CColor& defaultColor)
{
    size_t rem = str.size();
    const utf8proc_uint8_t* it = reinterpret_cast<const utf8proc_uint8_t*>(str.data());
    utf8proc_int32_t lCh = -1;
    m_glyphs.clear();
    m_glyphs.reserve(str.size());
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
        {
            adv += m_fontAtlas.lookupKern(lCh, ch);
            m_glyphs.emplace_back(adv, *glyph, defaultColor);
        }
        else
            m_glyphs.emplace_back(adv, *glyph, defaultColor);

        lCh = ch;
        rem -= sz;
        it += sz;

        if (m_glyphs.size() == m_capacity)
            break;
    }

    m_validSlots = 0;
}
void TextView::typesetGlyphs(const std::wstring& str, const Zeus::CColor& defaultColor)
{
    wchar_t lCh = -1;
    m_glyphs.clear();
    m_glyphs.reserve(str.size());
    int adv = 0;

    for (wchar_t ch : str)
    {
        if (ch == L'\n')
            break;

        const FontAtlas::Glyph* glyph = m_fontAtlas.lookupGlyph(ch);
        if (!glyph)
            continue;

        if (lCh != -1)
        {
            adv += DoKern(m_fontAtlas.lookupKern(lCh, ch), m_fontAtlas);
            m_glyphs.emplace_back(adv, *glyph, defaultColor);
        }
        else
            m_glyphs.emplace_back(adv, *glyph, defaultColor);

        lCh = ch;

        if (m_glyphs.size() == m_capacity)
            break;
    }

    m_validSlots = 0;
}

void TextView::colorGlyphs(const Zeus::CColor& newColor)
{
    for (RenderGlyph& glyph : m_glyphs)
        glyph.m_color = newColor;
    m_validSlots = 0;
}
void TextView::colorGlyphsTypeOn(const Zeus::CColor& newColor, float startInterval, float fadeTime)
{
}
void TextView::think()
{
}

void TextView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    View::draw(gfxQ);
    int pendingSlot = 1 << gfxQ->pendingDynamicSlot();
    if ((m_validSlots & pendingSlot) == 0)
    {
        m_glyphBuf->load(m_glyphs.data(), m_glyphs.size() * sizeof(RenderGlyph));
        m_validSlots |= pendingSlot;
    }
    gfxQ->setShaderDataBinding(m_shaderBinding);
    gfxQ->drawInstances(0, 4, m_glyphs.size());
}


}
