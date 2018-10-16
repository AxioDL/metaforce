#pragma once

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
    CScriptCameraHintTrigger(TUniqueId, bool, std::string_view name, const CEntityInfo&, const zeus::CVector3f&,
                             const zeus::CTransform&, bool, bool);

    void Accept(IVisitor& visitor);
};
}
