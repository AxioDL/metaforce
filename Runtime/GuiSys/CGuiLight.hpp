#ifndef __URDE_CGUILIGHT_HPP__
#define __URDE_CGUILIGHT_HPP__

#include "CGuiWidget.hpp"
#include "Graphics/CLight.hpp"

namespace urde
{
class CSimplePool;

class CGuiLight : public CGuiWidget
{
    ELightType xf8_type;
    float xfc_spotCutoff;
    float x100_distC;
    float x104_distL;
    float x108_distQ;
    float x10c_angleC;
    float x110_angleL;
    float x114_angleQ;
    u32 x118_loadedIdx;
    zeus::CColor x11c_color = zeus::CColor::skBlack;
public:
    ~CGuiLight();
    CGuiLight(const CGuiWidgetParms& parms, const CLight& light);
    FourCC GetWidgetTypeID() const { return FOURCC('LITE'); }

    CLight BuildLight() const;
    void SetIsVisible(bool vis);
    u32 GetLoadedIdx() const {return x118_loadedIdx;}
    const zeus::CColor& GetColor() const {return x11c_color;}

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    std::shared_ptr<CGuiLight> shared_from_this()
    { return std::static_pointer_cast<CGuiLight>(CGuiObject::shared_from_this()); }
};

}

#endif // __URDE_CGUILIGHT_HPP__
