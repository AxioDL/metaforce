#include "CGuiCamera.hpp"
#include "CGuiFrame.hpp"

namespace urde
{

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms,
                       float left, float right,
                       float top, float bottom,
                       float znear, float zfar)
: CGuiWidget(parms)
{
}

CGuiCamera::CGuiCamera(const CGuiWidgetParms& parms,
                       float fov, float aspect,
                       float znear, float zfar)
: CGuiWidget(parms)
{
}

CGuiCamera* CGuiCamera::Create(CGuiFrame* frame, CInputStream& in, bool flag)
{
    CGuiWidgetParms parms = ReadWidgetHeader(frame, in, flag);
    Projection proj = Projection(in.readUint32Big());
    CGuiCamera* ret = nullptr;
    switch (proj)
    {
    case Projection::Perspective:
    {
        float fov = in.readFloatBig();
        float aspect = in.readFloatBig();
        float znear = in.readFloatBig();
        float zfar = in.readFloatBig();
        ret = new CGuiCamera(parms, fov, aspect, znear, zfar);
        break;
    }
    case Projection::Orthographic:
    {
        float left = in.readFloatBig();
        float right = in.readFloatBig();
        float top = in.readFloatBig();
        float bottom = in.readFloatBig();
        float znear = in.readFloatBig();
        float zfar = in.readFloatBig();
        ret = new CGuiCamera(parms, left, right, top, bottom, znear, zfar);
        break;
    }
    break;
    }
    frame->SetFrameCamera(ret);
    return ret;
}

}
