#ifndef _CSCRIPTAREAATTRIBUTES
#define _CSCRIPTAREAATTRIBUTES

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CWorld.hpp"

class CScriptAreaAttributes : public CEntity {
  bool mShowSkybox : 1;
  EEnvFxType mEnvFx;
  float mEnvFxDensity;
  float mThermalHeat;
  float mXrayFogDistance;
  float mWorldLightingLevel;
  CAssetId mSkybox;
  EPhazonType mPhazon;

public:
  CScriptAreaAttributes(TUniqueId uid, const CEntityInfo& info, bool showSkybox, EEnvFxType fxType,
                        float envFxDensity, float thermalHeat, float xrayFogDistance,
                        float worldLightingLevel, CAssetId skybox, EPhazonType phazonType);

  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  DECLARE_ACCEPT;

  bool GetNeedsSky() const { return mShowSkybox; }
  bool GetNeedsEnvFx() const { return mEnvFx != kEFX_None; }
  CAssetId GetSkyModel() const { return mSkybox; }
  EEnvFxType GetEnvFxType() const { return mEnvFx; }
  float GetEnvFxDensity() const { return mEnvFxDensity; }
  float GetThermalHeat() const { return mThermalHeat; }
  float GetXRayFogDistance() const { return mXrayFogDistance; }
  float GetWorldLightingLevel() const { return mWorldLightingLevel; }
  EPhazonType GetPhazonType() const { return mPhazon; }
};

#endif // _CSCRIPTAREAATTRIBUTES
