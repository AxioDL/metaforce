#include "CAuiImagePane.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CAuiImagePane::CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, ResId tex0, ResId tex1,
                             rstl::reserved_vector<zeus::CVector3f, 4>&& coords,
                             rstl::reserved_vector<zeus::CVector2f, 4>&& uvs, bool initTex)
: CGuiWidget(parms), xc8_tex0(tex0), xcc_tex1(tex1), xe0_coords(std::move(coords)),
  x114_uvs(std::move(uvs))
{
    if (initTex)
        SetTextureID0(tex0, sp);
}

std::shared_ptr<CGuiWidget> CAuiImagePane::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
    in.readUint32Big();
    in.readUint32Big();
    in.readUint32Big();
    u32 coordCount = in.readUint32Big();
    rstl::reserved_vector<zeus::CVector3f, 4> coords;
    for (int i=0 ; i<coordCount ; ++i)
        coords.push_back(zeus::CVector3f::ReadBig(in));
    u32 uvCount = in.readUint32Big();
    rstl::reserved_vector<zeus::CVector2f, 4> uvs;
    for (int i=0 ; i<uvCount ; ++i)
        uvs.push_back(zeus::CVector2f::ReadBig(in));
    std::shared_ptr<CGuiWidget> ret = std::make_shared<CAuiImagePane>(parms, sp, -1, -1,
                                                                      std::move(coords), std::move(uvs), true);
    ret->ParseBaseInfo(frame, in, parms);
    return ret;
}

void CAuiImagePane::SetTextureID0(ResId tex, CSimplePool* sp)
{
    xc8_tex0 = tex;
    if (!sp)
        return;
    if (xc8_tex0 != -1)
        xb8_tex0Tok = sp->GetObj({FOURCC('TXTR'), xc8_tex0});
    else
        xb8_tex0Tok = TLockedToken<CTexture>();
}

void CAuiImagePane::SetAnimationParms(const zeus::CVector2f& vec, float interval, float duration)
{
    x138_ = vec;
    x140_interval = interval;
    x144_ = 0.f;
    x148_duration = duration;
}

}
