#include "CGuiStaticImage.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"

namespace urde
{

CGuiStaticImage::CGuiStaticImage
    (const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& c,
     EGuiTextureClampModeHorz, EGuiTextureClampModeVert,
     CGuiStaticImage::EMaterialType, u32, u32,
     std::vector<float>&&, bool)
: CGuiPane(parms, a, b, c)
{
}

CGuiStaticImage* CGuiStaticImage::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    float c = in.readFloatBig();
    float d = in.readFloatBig();
    std::vector<float> floats = {a, b, a, d, c, b, c, d};

    zeus::CVector3f vec;
    vec.readBig(in);
    u32 e = in.readUint32Big();
    u32 f = in.readUint32Big();
    u32 g = in.readUint32Big();

}

}
