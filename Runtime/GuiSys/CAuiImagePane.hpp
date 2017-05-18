#ifndef __URDE_CAUIIMAGEPANE_HPP__
#define __URDE_CAUIIMAGEPANE_HPP__

#include "CGuiWidget.hpp"
#include "CToken.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"

namespace urde
{
class CSimplePool;
class CTexture;

class CAuiImagePane : public CGuiWidget
{
    TLockedToken<CTexture> xb8_tex0Tok; // Used to be optional
    ResId xc8_tex0;
    ResId xcc_tex1;
    zeus::CVector2f xd0_uvBias0;
    zeus::CVector2f xd8_uvBias1;
    rstl::reserved_vector<zeus::CVector3f, 4> xe0_coords;
    rstl::reserved_vector<zeus::CVector2f, 4> x114_uvs;
    zeus::CVector2f x138_tileSize;
    float x140_interval = 0.f;
    float x144_frameTimer = 0.f;
    float x148_fadeDuration = 0.f;
    float x14c_deResFactor = 0.f;
    float x150_flashFactor = 0.f;
    struct Filters
    {
        CTexturedQuadFilterAlpha m_darkenerQuad;
        CTexturedQuadFilterAlpha m_flashQuad[2];
        CTexturedQuadFilterAlpha m_alphaQuad[2];
        CTexturedQuadFilterAlpha m_addQuad[2];
        Filters(TLockedToken<CTexture>& tex);
    };
    std::experimental::optional<Filters> m_filters;
    void DoDrawImagePane(const zeus::CColor& color, const CTexture& tex,
                         int frame, float blurAmt, bool noBlur, CTexturedQuadFilterAlpha& quad) const;
public:
    CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, ResId, ResId,
                  rstl::reserved_vector<zeus::CVector3f, 4>&& coords,
                  rstl::reserved_vector<zeus::CVector2f, 4>&& uvs, bool initTex);
    FourCC GetWidgetTypeID() const { return FOURCC('IMGP'); }
    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    void Update(float dt);
    void Draw(const CGuiWidgetDrawParms& params) const;
    bool GetIsFinishedLoadingWidgetSpecific() const;
    void SetTextureID0(ResId tex, CSimplePool* sp);
    void SetAnimationParms(const zeus::CVector2f& vec, float interval, float duration);
    void SetDeResFactor(float d) { x14c_deResFactor = d; }
    void SetFlashFactor(float t) { x150_flashFactor = t; }
};
}

#endif // __URDE_CAUIIMAGEPANE_HPP__
