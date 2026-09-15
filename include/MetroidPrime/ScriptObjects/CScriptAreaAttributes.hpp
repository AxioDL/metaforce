#ifndef _CSCRIPTAREAATTRIBUTES
#define _CSCRIPTAREAATTRIBUTES

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CWorld.hpp"

class CScriptAreaAttributes : public CEntity {
  bool x34_24_showSkybox : 1;
  EEnvFxType x38_envFx;
  float x3c_envFxDensity;
  float x40_thermalHeat;
  float x44_xrayFogDistance;
  float x48_worldLightingLevel;
  CAssetId x4c_skybox;
  EPhazonType x50_phazon;

public:
  CScriptAreaAttributes(TUniqueId uid, const CEntityInfo& info, bool showSkybox, EEnvFxType fxType,
                        float envFxDensity, float thermalHeat, float xrayFogDistance,
                        float worldLightingLevel, CAssetId skybox, EPhazonType phazonType);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  DECLARE_ACCEPT;

  bool GetNeedsSky() const { return x34_24_showSkybox; }
  bool GetNeedsEnvFx() const { return x38_envFx != kEFX_None; }
  CAssetId GetSkyModel() const { return x4c_skybox; }
  EEnvFxType GetEnvFxType() const { return x38_envFx; }
  float GetEnvFxDensity() const { return x3c_envFxDensity; }
  float GetThermalHeat() const { return x40_thermalHeat; }
  float GetXRayFogDistance() const { return x44_xrayFogDistance; }
  float GetWorldLightingLevel() const { return x48_worldLightingLevel; }
  EPhazonType GetPhazonType() const { return x50_phazon; }
};

#endif // _CSCRIPTAREAATTRIBUTES
