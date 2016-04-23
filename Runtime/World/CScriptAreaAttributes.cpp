#include "CScriptAreaAttributes.hpp"

namespace urde
{

CScriptAreaAttributes::CScriptAreaAttributes(TUniqueId uid, const CEntityInfo& info, bool showSkybox, EEnvFxType fxType,
                                             float envFxDensity, float thermalHeat, float xrayFogDistance,
                                             float worldLightingLevel, ResId skybox, EPhazonType phazonType)
    : CEntity(uid, info, true, std::string()),
      x34_24_showSkybox(showSkybox),
      x38_envFx(fxType),
      x3c_envFxDensity(envFxDensity),
      x40_thermalHeat(thermalHeat),
      x44_xrayFogDistance(xrayFogDistance),
      x48_worldLightingLevel(worldLightingLevel),
      x4c_skybox(skybox),
      x50_phazon(phazonType)
{
}

}
