#ifndef SPECTER_TEXTVIEW_HPP
#define SPECTER_TEXTVIEW_HPP

#include "View.hpp"
#include <boo/graphicsdev/GL.hpp>
#include <boo/graphicsdev/D3D.hpp>
#include <boo/graphicsdev/Metal.hpp>

#include "FontCache.hpp"

namespace Specter
{
class ViewSystem;

class TextView : public View
{
    boo::IGraphicsBufferD* m_glyphBuf;
    const FontAtlas& m_fontAtlas;
    boo::IVertexFormat* m_bgVtxFmt = nullptr; /* OpenGL only */

public:
    class System
    {
        friend class ViewSystem;
        friend class TextView;
        FontCache* m_fcache = nullptr;
        boo::IShaderPipeline* m_regular = nullptr;
        boo::IShaderPipeline* m_subpixel = nullptr;
        boo::IVertexFormat* m_vtxFmt = nullptr; /* Not OpenGL */

        void init(boo::GLDataFactory* factory, FontCache* fcache);
#if _WIN32
        void init(boo::ID3DDataFactory* factory, FontCache* fcache);
#elif BOO_HAS_METAL
        void init(boo::MetalDataFactory* factory, FontCache* fcache);
#endif
    };

    TextView(ViewSystem& system, FontTag font, size_t initGlyphCapacity=256);

    struct RenderGlyph
    {
        Zeus::CVector3f m_pos[4];
        Zeus::CMatrix4f m_mv;
        Zeus::CVector3f m_uv[4];
        Zeus::CColor m_color;

        RenderGlyph(int& adv, const FontAtlas::Glyph& glyph, Zeus::CColor defaultColor);
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
