#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/TGameTypes.hpp"

CScriptAreaAttributes::CScriptAreaAttributes(const TUniqueId uid, const CEntityInfo& info,
                                             const bool showSkybox, const EEnvFxType fxType,
                                             const float envFxDensity, const float thermalHeat,
                                             const float xrayFogDistance,
                                             const float worldLightingLevel, const CAssetId skybox,
                                             EPhazonType phazonType)
: CEntity(uid, info, true, rstl::string_l(""))
, mShowSkybox(showSkybox)
, mEnvFx(fxType)
, mEnvFxDensity(envFxDensity)
, mThermalHeat(thermalHeat)
, mXrayFogDistance(xrayFogDistance)
, mWorldLightingLevel(worldLightingLevel)
, mSkybox(skybox)
, mPhazon(phazonType) {}

void CScriptAreaAttributes::AcceptScriptMsg(const EScriptObjectMessage msg, const TUniqueId objId,
                                            CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);

  if (GetCurrentAreaId() == kInvalidAreaId) {
    return;
  }

  switch (msg) {
  case kSM_InitializedInArea: {
    stateMgr.World()->SetAreaAttributes(GetCurrentAreaId(), this);
    stateMgr.EnvFxManager()->SetFxDensity(500, mEnvFxDensity);
    break;
  }
  case kSM_Deleted: {
    if (!stateMgr.World()->Area(GetCurrentAreaId())->IsLoaded()) {
      break;
    }
    stateMgr.World()->SetAreaAttributes(GetCurrentAreaId(), nullptr);
    break;
  }
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptAreaAttributes)
