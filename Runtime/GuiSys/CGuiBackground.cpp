#include "CGuiBackground.hpp"
#include "CGuiAnimController.hpp"
#include "CGuiLogicalEventTrigger.hpp"

namespace urde
{

CGuiBackground::CGuiBackground(const CGuiWidgetParms& parms, float a, float b, const zeus::CVector3f& c,
                               EGuiTextureClampModeHorz horz, EGuiTextureClampModeVert vert,
                               CGuiStaticImage::EMaterialType mt, u32 d, u32 e,
                               std::vector<float>&& rect, bool flag)
: CGuiStaticImage(parms, a, b, c, horz, vert, mt, d, e, std::move(rect), flag)
{
}

CGuiBackground* CGuiBackground::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    float a = in.readFloatBig();
    float b = in.readFloatBig();
    float c = in.readFloatBig();
    float d = in.readFloatBig();
    std::vector<float> floats = {a, b, a, d, c, b, c, d};

    zeus::CVector3f vec;
    vec.readBig(in);
    in.readUint32Big();
    in.readUint32Big();
    in.readUint32Big();
    in.readUint32Big();
    return nullptr;
}

}
