#include "CScriptAreaAttributes.hpp"
#include "CEnvFxManager.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CScriptAreaAttributes::CScriptAreaAttributes(TUniqueId uid, const CEntityInfo& info, bool showSkybox, EEnvFxType fxType,
                                             float envFxDensity, float thermalHeat, float xrayFogDistance,
                                             float worldLightingLevel, CAssetId skybox, EPhazonType phazonType)
: CEntity(uid, info, true, std::string())
, x34_24_showSkybox(showSkybox)
, x38_envFx(fxType)
, x3c_envFxDensity(envFxDensity)
, x40_thermalHeat(thermalHeat)
, x44_xrayFogDistance(xrayFogDistance)
, x48_worldLightingLevel(worldLightingLevel)
, x4c_skybox(skybox)
, x50_phazon(phazonType) {}

void CScriptAreaAttributes::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptAreaAttributes::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  if (x4_areaId == kInvalidAreaId)
    return;

  if (msg == EScriptObjectMessage::InitializedInArea) {
    CGameArea* area = stateMgr.GetWorld()->GetArea(x4_areaId);
    area->SetAreaAttributes(this);
    stateMgr.GetEnvFxManager()->SetFxDensity(500, x3c_envFxDensity);
  } else if (msg == EScriptObjectMessage::Deleted) {
    CGameArea* area = stateMgr.GetWorld()->GetArea(x4_areaId);

    if (!area->IsPostConstructed())
      return;

    area->SetAreaAttributes(nullptr);
  }
}

} // namespace urde
