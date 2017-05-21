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
    struct SProjection
    {
        union
        {
            struct
            {
                float xbc_left;
                float xc0_right;
                float xc4_top;
                float xc8_bottom;
                float xcc_znear;
                float xd0_zfar;
            };
            struct
            {
                float xbc_fov;
                float xc0_aspect;
                float xc4_znear;
                float xc8_zfar;
            };
        };
        SProjection(float left, float right, float top, float bottom, float znear, float zfar)
        : xbc_left(left), xc0_right(right), xc4_top(top), xc8_bottom(bottom), xcc_znear(znear),
          xd0_zfar(zfar) {}
        SProjection(float fov, float aspect, float znear, float zfar)
        : xbc_fov(fov), xc0_aspect(aspect), xc4_znear(znear), xc8_zfar(zfar) {}
    };
private:
    EProjection xb8_projtype;
    SProjection m_proj;
public:
    CGuiCamera(const CGuiWidgetParms& parms, float left, float right,
                                             float top, float bottom,
                                             float znear, float zfar);
    CGuiCamera(const CGuiWidgetParms& parms, float fov, float aspect, float znear, float zfar);
    FourCC GetWidgetTypeID() const { return FOURCC('CAMR'); }

    static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

    zeus::CVector3f ConvertToScreenSpace(const zeus::CVector3f& vec) const;
    const SProjection& GetProjection() const { return m_proj; }
    void SetFov(float fov) { m_proj.xbc_fov = fov; }
    void Draw(const CGuiWidgetDrawParms& parms) const;

    std::shared_ptr<CGuiCamera> shared_from_this()
    { return std::static_pointer_cast<CGuiCamera>(CGuiObject::shared_from_this()); }
};

}

#endif // __URDE_CGUICAMERA_HPP__
