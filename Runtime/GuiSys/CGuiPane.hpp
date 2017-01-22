#ifndef __URDE_CGUIPANE_HPP__
#define __URDE_CGUIPANE_HPP__

#include "CGuiWidget.hpp"
#include "specter/View.hpp"

namespace urde
{

class CGuiPane : public CGuiWidget
{
protected:
    zeus::CVector2f xb8_dim;

    /* Originally a vert-buffer pointer for GX */
    std::vector<specter::View::TexShaderVert> x100_verts;
    // u32 x104_ = 4; /* vert count */

    zeus::CVector3f x108_scaleCenter;

public:
    CGuiPane(const CGuiWidgetParms& parms, const zeus::CVector2f& dim, const zeus::CVector3f& scaleCenter);
    FourCC GetWidgetTypeID() const {return FOURCC('PANE');}

    virtual void ScaleDimensions(const zeus::CVector3f& scale);
    virtual void SetDimensions(const zeus::CVector2f& dim, bool initVBO);
    virtual zeus::CVector2f GetDimensions() const;
    virtual void InitializeBuffers();
    virtual void WriteData(COutputStream& out, bool flag) const;

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

}

#endif // __URDE_CGUIPANE_HPP__
