#ifndef __URDE_CGUICAMERA_HPP__
#define __URDE_CGUICAMERA_HPP__

#include "CGuiWidget.hpp"

namespace urde
{

class CGuiCamera : public CGuiWidget
{
public:
    enum class Projection
    {
        Perspective,
        Orthographic
    };
private:
    Projection xf8_proj;
public:
    CGuiCamera(const CGuiWidgetParms& parms, float, float, float, float, float, float);
    CGuiCamera(const CGuiWidgetParms& parms, float fov, float aspect, float znear, float zfar);
    static CGuiCamera* Create(CGuiFrame* frame, CInputStream& in, bool flag);
};

}

#endif // __URDE_CGUICAMERA_HPP__
