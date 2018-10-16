#pragma once

#include "CActor.hpp"
#include "zeus/CRelAngle.hpp"
#include "zeus/COBBox.hpp"

namespace urde
{
class CScriptCameraPitchVolume : public CActor
{
    static const zeus::CVector3f skScaleFactor;
    zeus::COBBox xe8_obbox;
    zeus::CRelAngle x124_upPitch;
    zeus::CRelAngle x128_downPitch;
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
    CScriptCameraPitchVolume(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&,
                             const zeus::CTransform&, const zeus::CRelAngle&, const zeus::CRelAngle&, float);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager&);
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&);
    float GetUpPitch() const { return x124_upPitch; }
    float GetDownPitch() const { return x128_downPitch; }
    const zeus::CVector3f& GetScale() const { return x12c_scale; }
    float GetMaxInterpolationDistance() const { return x138_maxInterpDistance; }
    void Entered(CStateManager&);
    void Exited(CStateManager&);
};
}
