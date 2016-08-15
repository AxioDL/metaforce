#ifndef __URDE_CGUITEXTPANE_HPP__
#define __URDE_CGUITEXTPANE_HPP__

#include "CGuiPane.hpp"
#include "CGuiTextSupport.hpp"

namespace urde
{

class CGuiTextPane : public CGuiPane
{
    CGuiTextSupport x114_textSupport;
public:
    CGuiTextPane(const CGuiWidgetParms& parms, float xDim, float zDim, const zeus::CVector3f& vec,
                 ResId fontId, const CGuiTextProperties& props, const zeus::CColor& col1,
                 const zeus::CColor& col2, s32 padX, s32 padY);
    FourCC GetWidgetTypeID() const {return FOURCC('TXPN');}

    CGuiTextSupport* TextSupport() {return &x114_textSupport;}
    const CGuiTextSupport* GetTextSupport() const {return &x114_textSupport;}
    void Update(float dt);
    bool GetIsFinishedLoadingWidgetSpecific() const;
    std::vector<ResId> GetFontAssets() const {return {x114_textSupport.x5c_fontId};}
    void SetDimensions(const zeus::CVector2f& dim, bool initVBO);
    void ScaleDimensions(const zeus::CVector3f& scale);
    void Draw(const CGuiWidgetDrawParms& parms) const;

    static CGuiTextPane* Create(CGuiFrame* frame, CInputStream& in, bool);
};

}

#endif // __URDE_CGUITEXTPANE_HPP__
