#pragma once

#include "RetroTypes.hpp"
#include "CIOWin.hpp"
#include "CToken.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{
class CTexture;
class CDependencyGroup;

class CSlideShow : public CIOWin
{
public:
    enum class Phase
    {
        Zero,
        One,
        Two,
        Three,
        Four,
        Five
    };
    struct SSlideData
    {
        CSlideShow& x0_parent;
        u32 x4_ = -1;
        u32 x8_ = -1;

        std::experimental::optional<CTexturedQuadFilterAlpha> m_texQuad;
        zeus::CVector2f x18_vpOffset;
        zeus::CVector2f x20_vpSize;
        zeus::CVector2f x28_canvasSize;
        zeus::CColor x30_mulColor = zeus::CColor::skWhite;

        SSlideData(CSlideShow& parent) : x0_parent(parent)
        {
            x30_mulColor.a = 0.f;
        }

        void SetTexture(const TLockedToken<CTexture>& tex)
        {
            m_texQuad.emplace(EFilterType::Blend, tex);
        }
        bool IsLoaded() const
        {
            return m_texQuad && m_texQuad->GetTex().IsLoaded();
        }
        void Draw() const;
    };

private:
    Phase x14_phase = Phase::Zero;
    std::vector<TLockedToken<CDependencyGroup>> x18_galleryTXTRDeps;
    /*
    u32 x2c_ = 0;
    u32 x30_ = 0;
    u32 x34_ = 0;
    u32 x38_ = 0;
    u32 x3c_ = 0;
    u32 x40_ = 0;
    u32 x44_ = 0;
    u32 x48_ = -1;
    float x4c_ = 0.f;
    float x50_ = 0.f;
    float x54_ = 0.f;
    float x58_ = 0.f;
    */

    SSlideData x5c_slideA;
    SSlideData x90_slideB;

    std::unique_ptr<CGuiTextSupport> xc4_textA;
    std::unique_ptr<CGuiTextSupport> xc8_textB;
    /*
    u32 xcc_ = 0;
    u32 xd4_ = 0;
    u32 xd8_ = 0;
    u32 xdc_ = 0;
    u32 xe0_ = 0;
    */
    CSfxHandle xe4_;
    /*
    u32 xe8_ = 0;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    u32 xf4_ = 0;
    */
    std::vector<TLockedToken<CTexture>> xf8_stickTextures; /* (9 LStick, 9 CStick) */
    std::vector<CToken> x108_buttonTextures; /* (2L, 2R, 2B, 2Y) */
    /*
    u32 x11c_ = 0;
    u32 x120_ = 0;
    u32 x124_ = 0;
    float x128_ = 32.f;
    float x12c_ = 32.f;
    */
    float x130_;

    union
    {
        struct
        {
            bool x134_24_ : 1;
            bool x134_25_ : 1;
            bool x134_26_ : 1;
            bool x134_27_ : 1;
            bool x134_28_disableInput : 1;
            bool x134_29_ : 1;
            bool x134_30_ : 1;
            bool x134_31_ : 1;
            bool x135_24_ : 1;
        };
        u32 dummy = 0;
    };

    bool LoadTXTRDep(std::string_view name);
    static bool AreAllDepsLoaded(const std::vector<TLockedToken<CDependencyGroup>>& deps);

public:
    CSlideShow();
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
    bool GetIsContinueDraw() const {return false;}
    void Draw() const;

    static u32 SlideShowGalleryFlags();
};

}

