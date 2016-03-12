#ifndef __URDE_CGUIBACKGROUND_HPP__
#define __URDE_CGUIBACKGROUND_HPP__

#include "CGuiWidget.hpp"
#include "CGuiStaticImage.hpp"

namespace urde
{

class CGuiBackground : public CGuiStaticImage
{
public:
    CGuiBackground(const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& c,
                   EGuiTextureClampModeHorz horz, EGuiTextureClampModeVert vert,
                   CGuiStaticImage::EMaterialType mt, u32 d, u32 e,
                   std::vector<float>&& rect, bool flag);
    static CGuiBackground* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUIBACKGROUND_HPP__
