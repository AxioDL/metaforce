#ifndef SPECTER_TEXTVIEW_HPP
#define SPECTER_TEXTVIEW_HPP

#include "View.hpp"
#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/Metal.hpp>

#include "FontCache.hpp"

namespace Specter
{

class TextView : public View
{
    boo::IGraphicsBufferD* m_glyphBuf;
    int m_validDynamicSlots = 0;
    size_t m_curGlyphCapacity;
    const FontAtlas& m_fontAtlas;
    boo::IVertexFormat* m_vtxFmt = nullptr; /* OpenGL only */

public:
    class System
    {
        friend class TextView;
        boo::IGraphicsDataFactory* m_factory;
        FontCache& m_fcache;
        boo::IShaderPipeline* m_regular;
        boo::IShaderPipeline* m_subpixel;
        boo::IGraphicsBufferS* m_quadVBO;
        boo::IVertexFormat* m_vtxFmt = nullptr; /* Not OpenGL */

        System(boo::IGraphicsDataFactory* factory, FontCache& fcache)
        : m_factory(factory), m_fcache(fcache) {}
    };
    static System BuildTextSystem(boo::GLDataFactory* factory, FontCache& fcache);
#if _WIN32
    static Shaders BuildTextSystem(boo::ID3DDataFactory* factory, FontCache& fcache);
#elif BOO_HAS_METAL
    static Shaders BuildTextSystem(boo::MetalDataFactory* factory, FontCache& fcache);
#endif

    TextView(System& system, FontTag font, size_t initGlyphCapacity=256);

    struct RenderGlyph
    {
        Zeus::CMatrix4f m_mvp;
        Zeus::CVector3f m_uv[4];
        Zeus::CColor m_color;
    };
    std::vector<RenderGlyph>& accessGlyphs() {return m_glyphs;}
    void updateGlyphs() {m_validDynamicSlots = 0;}

    void typesetGlyphs(const std::string& str,
                       Zeus::CColor defaultColor=Zeus::CColor::skWhite);
    void typesetGlyphs(const std::wstring& str,
                       Zeus::CColor defaultColor=Zeus::CColor::skWhite);

    void colorGlyphs(Zeus::CColor newColor);
    void colorGlyphsTypeOn(Zeus::CColor newColor, float startInterval=0.2, float fadeTime=0.5);
    void think();

    void draw(boo::IGraphicsCommandQueue* gfxQ);

private:
    std::vector<RenderGlyph> m_glyphs;
};

}

#endif // SPECTER_TEXTVIEW_HPP
