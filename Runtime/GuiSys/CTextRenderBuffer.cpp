#include "CTextRenderBuffer.hpp"
#include "CFontImageDef.hpp"
#include "Graphics/CGraphicsPalette.hpp"
#include "Graphics/CGraphics.hpp"
#include "CRasterFont.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{

boo::IShaderPipeline* g_TextShaderPipeline = nullptr;
boo::IVertexFormat* g_TextVtxFmt = nullptr;

boo::IShaderPipeline* g_TextImageShaderPipeline = nullptr;
boo::IVertexFormat* g_TextImageVtxFmt = nullptr;

CTextRenderBuffer::CTextRenderBuffer(EMode mode)
: x0_mode(mode) {}

void CTextRenderBuffer::BooCharacterInstance::SetMetrics(const CGlyph& glyph,
                                                         const zeus::CVector2i& offset)
{
    m_pos[0].assign(offset.x, 0.f, offset.y);
    m_uv[0].assign(glyph.GetStartU(), glyph.GetStartV());

    m_pos[1].assign(offset.x + glyph.GetCellWidth(), 0.f, offset.y);
    m_uv[1].assign(glyph.GetEndU(), glyph.GetStartV());

    m_pos[2].assign(offset.x, 0.f, offset.y + glyph.GetCellHeight());
    m_uv[2].assign(glyph.GetStartU(), glyph.GetEndV());

    m_pos[3].assign(offset.x + glyph.GetCellWidth(), 0.f, offset.y + glyph.GetCellHeight());
    m_uv[3].assign(glyph.GetEndU(), glyph.GetEndV());
}

void CTextRenderBuffer::BooPrimitiveMark::SetOpacity(CTextRenderBuffer& rb, float opacity)
{
    switch (m_cmd)
    {
    case Command::CharacterRender:
    {
        BooFontCharacters& fc = rb.m_fontCharacters[m_bindIdx];
        BooCharacterInstance& inst = fc.m_charData[m_instIdx];
        inst.m_fontColor.a = opacity;
        inst.m_outlineColor.a = opacity;
        fc.m_dirty = true;
        break;
    }
    case Command::ImageRender:
    {
        BooImage& img = rb.m_images[m_bindIdx];
        img.m_imageData.m_color.a = opacity;
        img.m_dirty = true;
        break;
    }
    default: break;
    }
}

void CTextRenderBuffer::CommitResources()
{
    if (m_committed)
        return;
    m_committed = true;

    m_booToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(BooUniform), 1);

        for (BooFontCharacters& chs : m_fontCharacters)
        {
            chs.m_instBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex,
                                                 sizeof(BooCharacterInstance),
                                                 chs.m_charCount);
            boo::IVertexFormat* vFmt = g_TextVtxFmt;
            if (ctx.bindingNeedsVertexFormat())
            {
                boo::VertexElementDescriptor elems[] =
                {
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 0},
                    {chs.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 1},
                };
                vFmt = ctx.newVertexFormat(10, elems);
            }

            boo::IGraphicsBuffer* uniforms[] = {m_uniBuf};
            boo::ITexture* texs[] = {chs.m_font.GetObj()->GetTexture()->GetBooTexture()};
            chs.m_dataBinding = ctx.newShaderDataBinding(g_TextShaderPipeline, vFmt,
                                                         nullptr, chs.m_instBuf, nullptr,
                                                         1, uniforms, nullptr, 1, texs);
        }

        for (BooImage& img : m_images)
        {
            img.m_instBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(BooImageInstance), 1);
            boo::IVertexFormat* vFmt = g_TextImageVtxFmt;
            if (ctx.bindingNeedsVertexFormat())
            {
                boo::VertexElementDescriptor elems[] =
                {
                    {img.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 0},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 1},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 2},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::Position4 | boo::VertexSemantic::Instanced, 3},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 0},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 1},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 2},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::UV4 | boo::VertexSemantic::Instanced, 3},
                    {img.m_instBuf, nullptr, boo::VertexSemantic::Color | boo::VertexSemantic::Instanced, 0},
                };
                vFmt = ctx.newVertexFormat(9, elems);
            }

            boo::IGraphicsBuffer* uniforms[] = {m_uniBuf};
            img.m_dataBinding.reserve(img.m_imageDef.x4_texs.size());
            for (TToken<CTexture>& tex : img.m_imageDef.x4_texs)
            {
                boo::ITexture* texs[] = {tex->GetBooTexture()};
                img.m_dataBinding.push_back(ctx.newShaderDataBinding(g_TextImageShaderPipeline, vFmt,
                                                                     nullptr, img.m_instBuf, nullptr,
                                                                     1, uniforms, nullptr, 1, texs));
            }
        }
        return true;
    });
}

void CTextRenderBuffer::SetMode(EMode mode)
{
    if (mode == EMode::BufferFill)
    {
        m_images.reserve(m_imagesCount);
        for (BooFontCharacters& fc : m_fontCharacters)
            fc.m_charData.reserve(fc.m_charCount);
    }
    m_activeFontCh = -1;
}

void CTextRenderBuffer::SetPrimitiveOpacity(int idx, float opacity)
{
    m_primitiveMarks[idx].SetOpacity(*this, opacity);
}

void CTextRenderBuffer::Render(const zeus::CColor& col, float time) const
{
    ((CTextRenderBuffer*)this)->CommitResources();

    BooUniform uniforms = {CGraphics::GetPerspectiveProjectionMatrix(true) *
                           CGraphics::g_GXModelView.toMatrix4f(), col};
    m_uniBuf->load(&uniforms, sizeof(uniforms));

    for (const BooFontCharacters& chs : m_fontCharacters)
    {
        if (chs.m_dirty)
        {
            chs.m_instBuf->load(chs.m_charData.data(), sizeof(BooCharacterInstance) * chs.m_charData.size());
            ((BooFontCharacters&)chs).m_dirty = false;
        }
        CGraphics::SetShaderDataBinding(chs.m_dataBinding);
        CGraphics::DrawInstances(0, 4, chs.m_charData.size());
    }

    for (const BooImage& img : m_images)
    {
        if (img.m_dirty)
        {
            img.m_instBuf->load(&img.m_imageData, sizeof(BooImageInstance));
            ((BooImage&)img).m_dirty = false;
        }
        int idx = int(img.m_imageDef.x0_fps * time) % img.m_dataBinding.size();
        CGraphics::SetShaderDataBinding(img.m_dataBinding[idx]);
        CGraphics::DrawInstances(0, 4, 1);
    }
}

void CTextRenderBuffer::AddImage(const zeus::CVector2i& offset, const CFontImageDef& image)
{
    if (x0_mode == EMode::AllocTally)
        m_primitiveMarks.push_back({Command::ImageRender, m_imagesCount++, 0});
    else
        m_images.push_back({image});
}

void CTextRenderBuffer::AddCharacter(const zeus::CVector2i& offset, wchar_t ch,
                                     const zeus::CColor& color)
{
    if (m_activeFontCh == -1)
        return;
    BooFontCharacters& chs = m_fontCharacters[m_activeFontCh];
    if (x0_mode == EMode::AllocTally)
        m_primitiveMarks.push_back({Command::CharacterRender, m_activeFontCh, chs.m_charCount++});
    else
    {
        const CGlyph* glyph = chs.m_font.GetObj()->GetGlyph(ch);
        chs.m_charData.emplace_back();
        BooCharacterInstance& inst = chs.m_charData.back();
        inst.SetMetrics(*glyph, offset);
        inst.m_fontColor = m_main * color;
        inst.m_outlineColor = m_outline * color;
    }
}

void CTextRenderBuffer::AddPaletteChange(const zeus::CColor& main, const zeus::CColor& outline)
{
    m_main = main;
    m_outline = outline;
}

void CTextRenderBuffer::AddFontChange(const TToken<CRasterFont>& font)
{
    for (int i=0 ; i<m_fontCharacters.size() ; ++i)
    {
        BooFontCharacters& chs = m_fontCharacters[i];
        if (*chs.m_font.GetObjectTag() == *font.GetObjectTag())
        {
            m_activeFontCh = i;
            return;
        }
    }

    m_activeFontCh = m_fontCharacters.size();
    m_fontCharacters.push_back({font});
}

}
