#ifndef __URDE_CGUIPANE_HPP__
#define __URDE_CGUIPANE_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CGuiPane : public CGuiWidget
{
    float xf8_a;
    float xfc_b;
    u32 x100_ = 0;
    u32 x104_ = 4;
    zeus::CVector3f x108_vec;
public:
    CGuiPane(const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& vec);
    static CGuiPane* Create(CGuiFrame* frame, CInputStream& in, bool);

    virtual void ScaleDimensions(const zeus::CVector3f& scale);
    virtual void SetDimensions(const zeus::CVector2f& dim, bool flag);
    virtual const zeus::CVector3f& GetDimensions() const;
    virtual void InitializeBuffers();
    virtual void WriteData(COutputStream& out, bool flag) const;
};

}

#endif // __URDE_CGUIPANE_HPP__
