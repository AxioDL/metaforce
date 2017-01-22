#include "CAuiImagePane.hpp"

namespace urde
{

CAuiImagePane::CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, ResId, ResId,
                             rstl::reserved_vector<zeus::CVector3f, 4>&& coords,
                             rstl::reserved_vector<zeus::CVector2f, 4>&& uvs, bool)
: CGuiWidget(parms)
{

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

}
