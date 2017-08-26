#ifndef __URDE_WEAPONCOMMON_HPP__
#define __URDE_WEAPONCOMMON_HPP__

#include "RetroTypes.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"
#include <set>

namespace urde
{
class CAnimData;
class CToken;
class CPrimitive;
enum class EWeaponType
{
    None = -1,
    Power = 0,
    Ice = 1,
    Wave = 2,
    Plasma = 3,
    Bomb = 4,
    PowerBomb = 5,
    Missile = 6,
    BoostBall = 7,
    Phazon = 8,
    AI = 9,
    PoisonWater = 10,
    Lava = 11,
    Hot = 12,
    Unused1 = 13,
    OrangePhazon = 14
};

namespace NWeaponTypes
{

enum class EGunAnimType
{
    Zero,
    One,
    Two,
    Three,
    Four,
    FromMissile,
    ToMissile,
    Seven,
    MissileReload,
    FromBeam
};

void primitive_set_to_token_vector(const CAnimData& animData, const std::set<CPrimitive>& primSet,
                                   std::vector<CToken>& tokensOut, bool preLock);
void unlock_tokens(std::vector<CToken>& anims);
void lock_tokens(std::vector<CToken>& anims);
bool are_tokens_ready(const std::vector<CToken>& anims);
void get_token_vector(CAnimData& animData, int begin, int end, std::vector<CToken>& tokensOut, bool preLock);
void get_token_vector(CAnimData& animData, int animIdx, std::vector<CToken>& tokensOut, bool preLock);

}

}
#endif // __URDE_WEAPONCOMMON_HPP__
