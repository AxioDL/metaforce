#ifndef __URDE_CSCRIPTAREAATTRIBUTES_HPP__
#define __URDE_CSCRIPTAREAATTRIBUTES_HPP__

#include "CEntity.hpp"
#include "CEnvFxManager.hpp"

namespace urde
{
class CScriptAreaAttributes : public CEntity
{
    bool x34_24_showSkybox : 1;
    EEnvFxType x38_envFx;
    float x3c_envFxDensity;
    float x40_thermalHeat;
    float x44_xrayFogDistance;
    float x48_worldLightingLevel;
    ResId x4c_skybox;
    EPhazonType x50_phazon;

public:
    CScriptAreaAttributes(TUniqueId uid, const CEntityInfo& info, bool showSkybox, EEnvFxType fxType,
                          float envFxDensity, float thermalHeat, float xrayFogDistance, float worldLightingLevel,
                          ResId skybox, EPhazonType phazonType);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);

    bool GetNeedsSky() const;
    bool GetNeedsEnvFx() const;
    float GetEnvFxDensity() const;
    float GetThermalHeat() const;
    float GetWorldLightingLevel() const;
};
}

#endif // __URDE_CSCRIPTAREAATTRIBUTES_HPP__
