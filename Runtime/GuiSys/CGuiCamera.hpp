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
    union
    {
        struct
        {
            float xfc_fov;
            float x100_aspect;
            float x104_znear;
            float x108_zfar;
        };
        struct
        {
            float xfc_left;
            float x100_right;
            float x104_top;
            float x108_bottom;
            float x10c_znear;
            float x110_zfar;
        };
    };
public:
    CGuiCamera(const CGuiWidgetParms& parms, float left, float right,
                                             float top, float bottom,
                                             float znear, float zfar);
    CGuiCamera(const CGuiWidgetParms& parms, float fov, float aspect, float znear, float zfar);
    static CGuiCamera* Create(CGuiFrame* frame, CInputStream& in, bool flag);

    zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f& vec) const;
    void Draw(const CGuiWidgetDrawParms& parms) const;
};

}

#endif // __URDE_CGUICAMERA_HPP__
