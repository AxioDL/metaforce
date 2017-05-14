#ifndef __URDE_CAUIIMAGEPANE_HPP__
#define __URDE_CAUIIMAGEPANE_HPP__

#include "CGuiWidget.hpp"
#include "CToken.hpp"

namespace urde
{
class CSimplePool;
class CTexture;

class CAuiImagePane : public CGuiWidget
{
    TLockedToken<CTexture> xb8_tex0Tok; // Used to be optional
    ResId xc8_tex0;
    ResId xcc_tex1;
    zeus::CVector2f xd0_;
    zeus::CVector2f xd8_;
    rstl::reserved_vector<zeus::CVector3f, 4> xe0_coords;
    rstl::reserved_vector<zeus::CVector2f, 4> x114_uvs;
    zeus::CVector2f x138_;
    float x140_interval = 0.f;
    float x144_ = 0.f;
    float x148_duration = 0.f;
    float x14c_invAlpha = 0.f;
    float x150_interp = 0.f;
public:
    CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, ResId, ResId,
                  rstl::reserved_vector<zeus::CVector3f, 4>&& coords,
                  rstl::reserved_vector<zeus::CVector2f, 4>&& uvs, bool initTex);
    FourCC GetWidgetTypeID() const { return FOURCC('IMGP'); }
    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    void SetTextureID0(ResId tex, CSimplePool* sp);
    void SetAnimationParms(const zeus::CVector2f& vec, float interval, float duration);
    void SetInverseAlpha(float a) { x14c_invAlpha = a; }
    void ResetInterp() { x150_interp = 0.f; }
};
}

#endif // __URDE_CAUIIMAGEPANE_HPP__
