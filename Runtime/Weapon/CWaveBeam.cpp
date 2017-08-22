#include "CWaveBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

CWaveBeam::CWaveBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                     EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale)
{
    x21c_waveBeam = g_SimplePool->GetObj("WaveBeam");
    x228_wave2nd1 = g_SimplePool->GetObj("Wave2nd_1");
    x234_wave2nd2 = g_SimplePool->GetObj("Wave2nd_2");
    x240_wave2nd3 = g_SimplePool->GetObj("Wave2nd_3");
    x258_24 = false;
    x258_25 = false;
}

}