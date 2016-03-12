#ifndef __URDE_CGUISTATICIMAGE_HPP__
#define __URDE_CGUISTATICIMAGE_HPP__

#include "CGuiPane.hpp"

namespace urde
{

class CGuiStaticImage : public CGuiPane
{
public:
    enum class EMaterialType
    {
    };
    CGuiStaticImage(const CGuiWidgetParms& parms, float, float, const zeus::CVector3f&,
                    EGuiTextureClampModeHorz, EGuiTextureClampModeVert,
                    CGuiStaticImage::EMaterialType, u32, u32,
                    std::vector<float>&&, bool);
    static CGuiStaticImage* Create(CGuiFrame* frame, CInputStream& in, bool flag);
};

}

#endif // __URDE_CGUISTATICIMAGE_HPP__
