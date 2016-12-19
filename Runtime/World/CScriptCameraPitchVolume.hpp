#ifndef __URDE_CSCRIPTCAMERAPITCHVOLUME_HPP__
#define __URDE_CSCRIPTCAMERAPITCHVOLUME_HPP__

#include "CActor.hpp"
#include "zeus/CRelAngle.hpp"
#include "zeus/COBBox.hpp"

namespace urde
{
class CScriptCameraPitchVolume : public CActor
{
    static const zeus::CVector3f skScaleFactor;
    zeus::COBBox xe8_obbox;
    zeus::CRelAngle x124_;
    zeus::CRelAngle x128_;
    zeus::CVector3f x12c_scale;
    float x138_maxInterpDistance;

    union {
        struct
        {
            bool x13c_24_entered : 1;
            bool x13c_25_occupied : 1;
        };
        u32 _dummy = 0;
    };

public:
    CScriptCameraPitchVolume(TUniqueId, bool, const std::string&, const CEntityInfo&, const zeus::CVector3f&,
                             const zeus::CTransform&, const zeus::CRelAngle&, const zeus::CRelAngle&, float);

    void Think(float, CStateManager&);
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&);
    const zeus::CRelAngle& GetUpPitch() const;
    const zeus::CRelAngle& GetDownPitch() const;
    const zeus::CVector3f& GetScale() const;
    float GetMaxInterpolationDistance() const;
    void Entered(CStateManager&);
    void Exited(CStateManager&);
};
}
#endif // __URDE_CSCRIPTCAMERAPITCHVOLUME_HPP__
