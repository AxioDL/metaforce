#include "Specter/TextView.hpp"
#include "utf8proc.h"

namespace Specter
{
static LogVisor::LogModule Log("Specter::TextView");

TextView::System TextView::BuildTextSystem(boo::GLDataFactory* factory, FontCache& fcache)
{
    static const char* VS =
    "#version 330\n"
    "layout(location=0) in vec3 posIn;\n"
    "layout(location=1) in mat4 mvMtx;\n"
    "layout(location=5) in vec3 uvIn[4];\n"
    "layout(location=9) in vec4 colorIn;\n"
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
    "    gl_Position = mvMtx * vec4(posIn, 1.0);\n"
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

    static float Quad[] =
    {
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 0.0, 0.0,
    };

    System ret(factory, fcache);

    ret.m_regular =
    factory->newShaderPipeline(VS, FSReg, 1, "fontTex", 0, nullptr,
                               boo::BlendFactor::SrcAlpha, boo::BlendFactor::InvSrcAlpha,
                               true, true, false);

    ret.m_subpixel =
    factory->newShaderPipeline(VS, FSSubpixel, 1, "fontTex", 0, nullptr,
                               boo::BlendFactor::SrcColor1, boo::BlendFactor::InvSrcColor1,
                               true, true, false);

    ret.m_quadVBO =
    factory->newStaticBuffer(boo::BufferUse::Vertex, Quad, sizeof(Quad), 1);

    return ret;
}

#if _WIN32
TextView::Shaders TextView::BuildTextSystem(boo::ID3DDataFactory* factory, FontCache& fcache)
{
}

#elif BOO_HAS_METAL
TextView::Shaders TextView::BuildTextSystem(boo::MetalDataFactory* factory, FontCache& fcache)
{
}
#endif

TextView::TextView(System& system, FontTag font, size_t initGlyphCapacity)
: m_curGlyphCapacity(initGlyphCapacity),
  m_fontAtlas(system.m_fcache.lookupAtlas(font))
{
    m_glyphBuf =
    system.m_factory->newDynamicBuffer(boo::BufferUse::Vertex,
                                       sizeof(RenderGlyph), initGlyphCapacity);

    if (!system.m_vtxFmt)
    {
        boo::VertexElementDescriptor vdescs[] =
        {
            {system.m_quadVBO, nullptr, boo::VertexSemantic::Position},
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
        m_vtxFmt = system.m_factory->newVertexFormat(10, vdescs);
    }

    m_glyphs.reserve(initGlyphCapacity);
}

void TextView::typesetGlyphs(const std::string& str, Zeus::CColor defaultColor)
{
    size_t rem = str.size();
    const utf8proc_uint8_t* it = str.data();
    while (rem)
    {
        utf8proc_int32_t ch;
        utf8proc_ssize_t sz = utf8proc_iterate(it, -1, &ch);
        if (sz < 0)
            Log.report(LogVisor::FatalError, "invalid UTF-8 char");

        m_fontAtlas

        rem -= sz;
        it += sz;
    }
}
void TextView::typesetGlyphs(const std::wstring& str, Zeus::CColor defaultColor)
{
}

void TextView::colorGlyphs(Zeus::CColor newColor)
{
}
void TextView::colorGlyphsTypeOn(Zeus::CColor newColor, float startInterval, float fadeTime)
{
}
void TextView::think()
{
}

void TextView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
}


}
