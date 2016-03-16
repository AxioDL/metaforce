#include "CGuiPane.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"

namespace urde
{

CGuiPane::CGuiPane(const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& vec)
: CGuiWidget(parms), xf8_a(a), xfc_b(b), x108_vec(vec)
{
    InitializeBuffers();
}

void CGuiPane::ScaleDimensions(const zeus::CVector3f& scale)
{
}

void CGuiPane::SetDimensions(const zeus::CVector2f& dim, bool flag)
{
}

const zeus::CVector3f& CGuiPane::GetDimensions() const
{
}

void CGuiPane::InitializeBuffers()
{
}

void CGuiPane::WriteData(COutputStream& out, bool flag) const
{
}

CGuiPane* CGuiPane::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    zeus::CVector3f vec;
    vec.readBig(in);
    return new CGuiPane(parms, a, b, vec);
}

}
