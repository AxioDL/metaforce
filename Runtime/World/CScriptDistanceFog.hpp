#pragma once

#include "Graphics/CGraphics.hpp"
#include "CEntity.hpp"

namespace urde
{
class CScriptDistanceFog : public CEntity
{
    ERglFogMode x34_mode;
    zeus::CColor x38_color;
    zeus::CVector2f x3c_range;
    float x44_colorDelta;
    zeus::CVector2f x48_rangeDelta;
    float x50_thermalTarget;
    float x54_thermalSpeed;
    float x58_xrayTarget;
    float x5c_xraySpeed;
    bool x60_explicit;
    bool x61_nonZero;
public:
    CScriptDistanceFog(TUniqueId, std::string_view, const CEntityInfo&, ERglFogMode,
                       const zeus::CColor&, const zeus::CVector2f&, float, const zeus::CVector2f&,
                       bool, bool, float, float, float, float);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};
}

