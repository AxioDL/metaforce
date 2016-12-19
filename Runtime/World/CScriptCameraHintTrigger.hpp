#ifndef __URDE_CSCRIPTCAMERAHINTTRIGGER_HPP__
#define __URDE_CSCRIPTCAMERAHINTTRIGGER_HPP__

#include "CActor.hpp"
#include "zeus/COBBox.hpp"

namespace urde
{
class CScriptCameraHintTrigger : public CActor
{
    zeus::COBBox xe8_obb;
    zeus::CVector3f x124_scale;
    union {
        struct
        {
            bool x130_24_ : 1;
            bool x130_25_ : 1;
            bool x130_26_ : 1;
            bool x130_27_ : 1;
        };
        u8 _dummy = 0;
    };

public:
    CScriptCameraHintTrigger(TUniqueId, bool, const std::string& name, const CEntityInfo&, const zeus::CVector3f&,
                             const zeus::CTransform&, bool, bool);
};
}
#endif // __URDE_CSCRIPTCAMERAHINTTRIGGER_HPP__
