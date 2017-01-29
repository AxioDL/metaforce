#ifndef __URDE_CTEXTRENDERBUFFER_HPP__
#define __URDE_CTEXTRENDERBUFFER_HPP__

#include "zeus/CColor.hpp"
#include "zeus/CVector2i.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CMatrix4f.hpp"
#include "CToken.hpp"
#include "CFontImageDef.hpp"
#include "RetroTypes.hpp"
#include "CGuiWidget.hpp"
#include "Graphics/Shaders/CTextSupportShader.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{
class CGraphicsPalette;
class CRasterFont;
class CGlyph;
class CTextExecuteBuffer;

using CTextColor = zeus::CColor;

class CTextRenderBuffer
{
    friend class CGuiTextSupport;
    friend class CTextSupportShader;
public:
    enum class Command
    {
        CharacterRender,
        ImageRender,
        FontChange,
        PaletteChange
    };
#if 0
    struct Primitive
    {
        CTextColor x0_color1;
        Command x4_command;
        u16 x8_xPos;
        u16 xa_zPos;
        wchar_t xc_glyph;
        u8 xe_imageIndex;
    };
#endif
    enum class EMode
    {
        AllocTally,
        BufferFill
    };

private:
    EMode x0_mode;
#if 0
    std::vector<TToken<CRasterFont>> x4_fonts;
    std::vector<CFontImageDef> x14_images;
    std::vector<int> x24_primOffsets;
    std::vector<char> x34_bytecode;
    u32 x44_blobSize = 0;
    u32 x48_curBytecodeOffset = 0;
    u8 x4c_activeFont;
    u32 x50_paletteCount = 0;
    std::array<std::unique_ptr<CGraphicsPalette>, 64> x54_palettes;
    u32 x254_nextPalette = 0;

#else
    /* Boo-specific text-rendering functionality */
    hecl::UniformBufferPool<CTextSupportShader::Uniform>::Token m_uniBuf;

    struct BooFontCharacters
    {
        TLockedToken<CRasterFont> m_font;
        hecl::VertexBufferPool<CTextSupportShader::CharacterInstance>::Token m_instBuf;
        boo::IShaderDataBinding* m_dataBinding = nullptr;
        std::vector<CTextSupportShader::CharacterInstance> m_charData;
        u32 m_charCount = 0;
        bool m_dirty = true;
        BooFontCharacters(const CToken& token)
        : m_font(token) {}
    };
    std::vector<BooFontCharacters> m_fontCharacters;

    struct BooImage
    {
        CFontImageDef m_imageDef;
        hecl::VertexBufferPool<CTextSupportShader::ImageInstance>::Token m_instBuf;
        std::vector<boo::IShaderDataBinding*> m_dataBinding;
        CTextSupportShader::ImageInstance m_imageData;
        bool m_dirty = true;
        BooImage(const CFontImageDef& imgDef, const zeus::CVector2i& offset);
    };
    std::vector<BooImage> m_images;

    boo::GraphicsDataToken m_booToken;

    struct BooPrimitiveMark
    {
        Command m_cmd;
        u32 m_bindIdx;
        u32 m_instIdx;
        void SetOpacity(CTextRenderBuffer& rb, float opacity);
    };
    std::vector<BooPrimitiveMark> m_primitiveMarks;
    u32 m_imagesCount = 0;
    u32 m_activeFontCh = -1;

    zeus::CColor m_main;
    zeus::CColor m_outline;

    CGuiWidget::EGuiModelDrawFlags m_drawFlags;

    bool m_committed = false;
    void CommitResources();
#endif

public:
    CTextRenderBuffer(EMode mode, CGuiWidget::EGuiModelDrawFlags df);
#if 0
    void SetPrimitive(const Primitive&, int);
    Primitive GetPrimitive(int) const;
    void GetOutStream();
    void VerifyBuffer();
    int GetMatchingPaletteIndex(const CGraphicsPalette& palette);
    CGraphicsPalette* GetNextAvailablePalette();
    void AddPaletteChange(const CGraphicsPalette& palette);
#else
    void SetPrimitiveOpacity(int idx, float opacity);
    u32 GetPrimitiveCount() const { return m_primitiveMarks.size(); }
#endif
    void SetMode(EMode mode);
    void Render(const zeus::CColor& col, float) const;
    void AddImage(const zeus::CVector2i& offset, const CFontImageDef& image);
    void AddCharacter(const zeus::CVector2i& offset, char16_t ch, const zeus::CColor& color);
    void AddPaletteChange(const zeus::CColor& main, const zeus::CColor& outline);
    void AddFontChange(const TToken<CRasterFont>& font);

    bool HasSpaceAvailable(const zeus::CVector2i& origin, const zeus::CVector2i& extent) const;
    std::pair<zeus::CVector2i, zeus::CVector2i> AccumulateTextBounds() const;
};

}

#endif // __URDE_CTEXTRENDERBUFFER_HPP__
