#ifndef __URDE_CGUILIGHT_HPP__
#define __URDE_CGUILIGHT_HPP__

#include "CGuiWidget.hpp"
#include "Graphics/CLight.hpp"

namespace urde
{

class CGuiLight : public CGuiWidget
{
    u32 x118_loadedIdx = 0;
public:
    CGuiLight(const CGuiWidgetParms& parms, const CLight& light);

    CLight BuildLight() const;
    u32 GetLoadedIdx() const {return x118_loadedIdx;}

    static CGuiLight* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUILIGHT_HPP__
