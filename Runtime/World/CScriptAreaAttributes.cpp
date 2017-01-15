#include "CScriptAreaAttributes.hpp"
#include "CEnvFxManager.hpp"
#include "CStateManager.hpp"
#include "CWorld.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptAreaAttributes::CScriptAreaAttributes(TUniqueId uid, const CEntityInfo& info, bool showSkybox, EEnvFxType fxType,
                                             float envFxDensity, float thermalHeat, float xrayFogDistance,
                                             float worldLightingLevel, ResId skybox, EPhazonType phazonType)
: CEntity(uid, info, true, std::string())
, x34_24_showSkybox(showSkybox)
, x38_envFx(fxType)
, x3c_envFxDensity(envFxDensity)
, x40_thermalHeat(thermalHeat)
, x44_xrayFogDistance(xrayFogDistance)
, x48_worldLightingLevel(worldLightingLevel)
, x4c_skybox(skybox)
, x50_phazon(phazonType)
{
}

void CScriptAreaAttributes::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptAreaAttributes::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    AcceptScriptMsg(msg, objId, stateMgr);
    if (x4_areaId == kInvalidAreaId)
        return;

    if (msg == EScriptObjectMessage::InternalMessage13)
    {
        CGameArea* area = stateMgr.WorldNC()->GetArea(x4_areaId);
        area->SetAreaAttributes(this);
        stateMgr.GetEnvFxManager()->SetFxDensity(500, x3c_envFxDensity);
    }
    else if (msg >= EScriptObjectMessage::InternalMessage12)
    {
        CGameArea* area = stateMgr.WorldNC()->GetArea(x4_areaId);

        if (!area->IsPostConstructed())
            return;

        area->SetAreaAttributes(nullptr);
    }
}

bool CScriptAreaAttributes::GetNeedsSky() const { return x34_24_showSkybox; }

bool CScriptAreaAttributes::GetNeedsEnvFx() const { return x38_envFx != EEnvFxType::None; }

float CScriptAreaAttributes::GetThermalHeat() const { return x40_thermalHeat; }

float CScriptAreaAttributes::GetWorldLightingLevel() const { return x48_worldLightingLevel; }
}
