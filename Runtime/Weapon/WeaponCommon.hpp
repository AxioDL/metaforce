#pragma once

#include "RetroTypes.hpp"
#include "DataSpec/DNACommon/Tweaks/ITweakPlayerGun.hpp"
#include "Audio/CSfxManager.hpp"
#include <set>
#include <Runtime/CPlayerState.hpp>

namespace urde
{
class CAnimData;
class CToken;
class CPrimitive;
class CStateManager;

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
    Heat = 12,
    Unused1 = 13,
    OrangePhazon = 14
};

enum class EProjectileAttrib
{
    None = 0,
    PartialCharge = (1 << 0),
    PlasmaProjectile = (1 << 1),
    Charged = (1 << 2),
    Ice = (1 << 3),
    Wave = (1 << 4),
    Plasma = (1 << 5),
    Phazon = (1 << 6),
    ComboShot = (1 << 7),
    Bombs = (1 << 8),
    PowerBombs = (1 << 9),
    BigProjectile = (1 << 10),
    ArmCannon = (1 << 11),
    BigStrike = (1 << 12),
    DamageFalloff = (1 << 13),
    StaticInterference = (1 << 14),
    PlayerUnFreeze = (1 << 15),
    ParticleOPTS = (1 << 16),
    KeepInCinematic = (1 << 17),
};
ENABLE_BITWISE_ENUM(EProjectileAttrib)

namespace NWeaponTypes
{

enum class EGunAnimType
{
    BasePosition,
    Shoot,
    ChargeUp,
    ChargeLoop,
    ChargeShoot,
    FromMissile,
    ToMissile,
    MissileShoot,
    MissileReload,
    FromBeam,
    ToBeam
};

void primitive_set_to_token_vector(const CAnimData& animData, const std::set<CPrimitive>& primSet,
                                   std::vector<CToken>& tokensOut, bool preLock);
void unlock_tokens(std::vector<CToken>& anims);
void lock_tokens(std::vector<CToken>& anims);
bool are_tokens_ready(const std::vector<CToken>& anims);
void get_token_vector(const CAnimData& animData, int begin, int end, std::vector<CToken>& tokensOut, bool preLock);
void get_token_vector(const CAnimData& animData, int animIdx, std::vector<CToken>& tokensOut, bool preLock);
void do_sound_event(std::pair<u16, CSfxHandle>& sfxHandle, float& pitch, bool doPitchBend, u32 soundId,
                    float weight, u32 flags, float falloff, float maxDist, float minVol, float maxVol,
                    const zeus::CVector3f& posToCam, const zeus::CVector3f& pos, TAreaId aid,
                    CStateManager& mgr);
CAssetId get_asset_id_from_name(const char* name);
CPlayerState::EPlayerSuit get_current_suit(const CStateManager& mgr);
CSfxHandle play_sfx(u16 sfx, bool underwater, bool looped, float pan);

}

}
