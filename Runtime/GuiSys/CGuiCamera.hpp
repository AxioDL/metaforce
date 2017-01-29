#ifndef __URDE_CGUICAMERA_HPP__
#define __URDE_CGUICAMERA_HPP__

#include "CGuiWidget.hpp"

namespace urde
{
class CSimplePool;

class CGuiCamera : public CGuiWidget
{
public:
    enum class EProjection
    {
        Perspective,
        Orthographic
    };
private:
    EProjection xf8_proj;
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
    FourCC GetWidgetTypeID() const { return FOURCC('CAMR'); }

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f& vec) const;
    void Draw(const CGuiWidgetDrawParms& parms) const;

    std::shared_ptr<CGuiCamera> shared_from_this()
    { return std::static_pointer_cast<CGuiCamera>(CGuiObject::shared_from_this()); }
};

}

#endif // __URDE_CGUICAMERA_HPP__
