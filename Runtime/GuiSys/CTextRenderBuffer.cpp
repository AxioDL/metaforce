#include "CTextRenderBuffer.hpp"
#include "CFontImageDef.hpp"
#include "Graphics/CGraphicsPalette.hpp"
#include "Graphics/CGraphics.hpp"
#include "CRasterFont.hpp"
#include "Graphics/CTexture.hpp"
#include "CTextExecuteBuffer.hpp"
#include "CFontRenderState.hpp"
#include "CInstruction.hpp"
#include "Graphics/Shaders/CTextSupportShader.hpp"

namespace urde
{

CTextRenderBuffer::CTextRenderBuffer(EMode mode, CGuiWidget::EGuiModelDrawFlags df)
: x0_mode(mode), m_drawFlags(df) {}

CTextRenderBuffer::BooImage::BooImage(const CFontImageDef& imgDef, const zeus::CVector2i& offset)
: m_imageDef(imgDef)
{
    m_imageData.SetMetrics(imgDef, offset);
}

void CTextRenderBuffer::BooPrimitiveMark::SetOpacity(CTextRenderBuffer& rb, float opacity)
{
    switch (m_cmd)
    {
    case Command::CharacterRender:
    {
        BooFontCharacters& fc = rb.m_fontCharacters[m_bindIdx];
        CTextSupportShader::CharacterInstance& inst = fc.m_charData[m_instIdx];
        inst.m_mulColor.a = opacity;
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

    /* Ensure font textures are ready outside transaction */
    for (BooFontCharacters& chs : m_fontCharacters)
        chs.m_font->GetTexture();

    CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_uniBuf = CTextSupportShader::s_Uniforms.allocateBlock(CGraphics::g_BooFactory);
        auto uBufInfo = m_uniBuf.getBufferInfo();
        decltype(uBufInfo) uBufInfo2;
        if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw)
        {
            m_uniBuf2 = CTextSupportShader::s_Uniforms.allocateBlock(CGraphics::g_BooFactory);
            uBufInfo2 = m_uniBuf2.getBufferInfo();
        }

        for (BooFontCharacters& chs : m_fontCharacters)
        {
            chs.m_instBuf = CTextSupportShader::s_CharInsts.allocateBlock(CGraphics::g_BooFactory, chs.m_charCount);
            auto iBufInfo = chs.m_instBuf.getBufferInfo();

            boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {uBufInfo.first.get()};
            boo::PipelineStage unistages[] = {boo::PipelineStage::Vertex};
            size_t unioffs[] = {size_t(uBufInfo.second)};
            size_t unisizes[] = {sizeof(CTextSupportShader::Uniform)};
            boo::ObjToken<boo::ITexture> texs[] = {chs.m_font->GetTexture()};
            chs.m_dataBinding = ctx.newShaderDataBinding(CTextSupportShader::SelectTextPipeline(m_drawFlags),
                                                         nullptr, iBufInfo.first.get(), nullptr,
                                                         1, uniforms, unistages, unioffs,
                                                         unisizes, 1, texs, nullptr, nullptr, 0, iBufInfo.second);

            if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw)
            {
                uniforms[0] = uBufInfo2.first.get();
                unioffs[0] = size_t(uBufInfo2.second);
                chs.m_dataBinding2 = ctx.newShaderDataBinding(CTextSupportShader::GetTextAdditiveOverdrawPipeline(),
                                                              nullptr, iBufInfo.first.get(), nullptr,
                                                              1, uniforms, unistages, unioffs,
                                                              unisizes, 1, texs, nullptr, nullptr, 0, iBufInfo.second);
            }
        }

        for (BooImage& img : m_images)
        {
            img.m_instBuf = CTextSupportShader::s_ImgInsts.allocateBlock(CGraphics::g_BooFactory, 1);
            auto iBufInfo = img.m_instBuf.getBufferInfo();

            boo::ObjToken<boo::IGraphicsBuffer> uniforms[] = {uBufInfo.first.get()};
            boo::PipelineStage unistages[] = {boo::PipelineStage::Vertex};
            size_t unioffs[] = {size_t(uBufInfo.second)};
            size_t unisizes[] = {sizeof(CTextSupportShader::Uniform)};
            img.m_dataBinding.reserve(img.m_imageDef.x4_texs.size());
            for (TToken<CTexture>& tex : img.m_imageDef.x4_texs)
            {
                boo::ObjToken<boo::ITexture> texs[] = {tex->GetBooTexture()};
                texs[0]->setClampMode(boo::TextureClampMode::ClampToEdge);
                img.m_dataBinding.push_back(ctx.newShaderDataBinding(CTextSupportShader::SelectImagePipeline(m_drawFlags),
                                                                     nullptr, iBufInfo.first.get(), nullptr,
                                                                     1, uniforms, unistages, unioffs,
                                                                     unisizes, 1, texs, nullptr, nullptr, 0, iBufInfo.second));
            }

            if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw)
            {
                uniforms[0] = uBufInfo2.first.get();
                unioffs[0] = size_t(uBufInfo2.second);
                img.m_dataBinding2.reserve(img.m_imageDef.x4_texs.size());
                for (TToken<CTexture>& tex : img.m_imageDef.x4_texs)
                {
                    boo::ObjToken<boo::ITexture> texs[] = {tex->GetBooTexture()};
                    img.m_dataBinding2.push_back(ctx.newShaderDataBinding(CTextSupportShader::GetImageAdditiveOverdrawPipeline(),
                                                                          nullptr, iBufInfo.first.get(), nullptr,
                                                                          1, uniforms, unistages, unioffs,
                                                                          unisizes, 1, texs, nullptr, nullptr, 0, iBufInfo.second));
                }
            }
        }
        return true;
    } BooTrace);
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
    x0_mode = mode;
}

void CTextRenderBuffer::SetPrimitiveOpacity(int idx, float opacity)
{
    m_primitiveMarks[idx].SetOpacity(*this, opacity);
}

void CTextRenderBuffer::Render(const zeus::CColor& col, float time) const
{
    const_cast<CTextRenderBuffer*>(this)->CommitResources();

    zeus::CMatrix4f mv = CGraphics::g_GXModelView.toMatrix4f();
    zeus::CMatrix4f proj = CGraphics::GetPerspectiveProjectionMatrix(true);
    zeus::CMatrix4f mat = proj * mv;

    const_cast<CTextRenderBuffer*>(this)->m_uniBuf.access() =
        CTextSupportShader::Uniform{mat, col};
    if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw)
    {
        zeus::CColor colPremul = col * col.a;
        colPremul.a = col.a;
        const_cast<CTextRenderBuffer*>(this)->m_uniBuf2.access() =
            CTextSupportShader::Uniform{mat, colPremul};
    }

    for (const BooFontCharacters& chs : m_fontCharacters)
    {
        if (chs.m_charData.size())
        {
            if (chs.m_dirty)
            {
                memmove(const_cast<BooFontCharacters&>(chs).m_instBuf.access(),
                        chs.m_charData.data(), sizeof(CTextSupportShader::CharacterInstance) *
                        chs.m_charData.size());
                const_cast<BooFontCharacters&>(chs).m_dirty = false;
            }
            CGraphics::SetShaderDataBinding(chs.m_dataBinding);
            CGraphics::DrawInstances(0, 4, chs.m_charData.size());
            if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw)
            {
                CGraphics::SetShaderDataBinding(chs.m_dataBinding2);
                CGraphics::DrawInstances(0, 4, chs.m_charData.size());
            }
        }
    }

    for (const BooImage& img : m_images)
    {
        if (img.m_dirty)
        {
            *const_cast<BooImage&>(img).m_instBuf.access() = img.m_imageData;
            const_cast<BooImage&>(img).m_dirty = false;
        }
        int idx = int(img.m_imageDef.x0_fps * time) % img.m_dataBinding.size();
        CGraphics::SetShaderDataBinding(img.m_dataBinding[idx]);
        CGraphics::DrawInstances(0, 4, 1);
        if (m_drawFlags == CGuiWidget::EGuiModelDrawFlags::AlphaAdditiveOverdraw)
        {
            CGraphics::SetShaderDataBinding(img.m_dataBinding2[idx]);
            CGraphics::DrawInstances(0, 4, 1);
        }
    }
}

void CTextRenderBuffer::AddImage(const zeus::CVector2i& offset, const CFontImageDef& image)
{
    if (x0_mode == EMode::AllocTally)
        m_primitiveMarks.push_back({Command::ImageRender, m_imagesCount++, 0});
    else
        m_images.push_back({image, offset});
}

void CTextRenderBuffer::AddCharacter(const zeus::CVector2i& offset, char16_t ch,
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
        CTextSupportShader::CharacterInstance& inst = chs.m_charData.back();
        inst.SetMetrics(*glyph, offset);
        inst.m_fontColor = m_main * color;
        inst.m_outlineColor = m_outline * color;
        inst.m_mulColor = zeus::CColor::skWhite;
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

bool CTextRenderBuffer::HasSpaceAvailable(const zeus::CVector2i& origin,
                                          const zeus::CVector2i& extent) const
{
    std::pair<zeus::CVector2i, zeus::CVector2i> bounds = AccumulateTextBounds();
    if (bounds.first.x > bounds.second.x)
        return true;

    if (0 < origin.y)
        return false;

    zeus::CVector2i size = bounds.second - bounds.first;
    return size.y <= extent.y;
}

std::pair<zeus::CVector2i, zeus::CVector2i> CTextRenderBuffer::AccumulateTextBounds() const
{
    std::pair<zeus::CVector2i, zeus::CVector2i> ret = std::make_pair(zeus::CVector2i{INT_MAX, INT_MAX},
                                                                     zeus::CVector2i{INT_MIN, INT_MIN});

    for (const BooFontCharacters& chars : m_fontCharacters)
    {
        for (const CTextSupportShader::CharacterInstance& charInst : chars.m_charData)
        {
            ret.first.x = std::min(ret.first.x, int(charInst.m_pos[0].x));
            ret.first.y = std::min(ret.first.y, int(charInst.m_pos[0].z));
            ret.second.x = std::max(ret.second.x, int(charInst.m_pos[3].x));
            ret.second.y = std::max(ret.second.y, int(charInst.m_pos[3].z));
        }
    }

    for (const BooImage& imgs : m_images)
    {
        ret.first.x = std::min(ret.first.x, int(imgs.m_imageData.m_pos[0].x));
        ret.first.y = std::min(ret.first.y, int(imgs.m_imageData.m_pos[0].z));
        ret.second.x = std::max(ret.second.x, int(imgs.m_imageData.m_pos[3].x));
        ret.second.y = std::max(ret.second.y, int(imgs.m_imageData.m_pos[3].z));
    }

    return ret;
}

}
