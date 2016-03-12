#ifndef __URDE_CGUILIGHT_HPP__
#define __URDE_CGUILIGHT_HPP__

#include "CGuiWidget.hpp"
#include "Graphics/CLight.hpp"

namespace urde
{

class CGuiLight : public CGuiWidget
{
public:
    CGuiLight(const CGuiWidgetParms& parms, const CLight& light);
    static CGuiLight* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUILIGHT_HPP__
