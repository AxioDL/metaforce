#ifndef __URDE_CPATTERNEDINFO_HPP__
#define __URDE_CPATTERNEDINFO_HPP__

#include "RetroTypes.hpp"
#include "CDamageInfo.hpp"
#include "CHealthInfo.hpp"
#include "CDamageVulnerability.hpp"
#include "CAnimationParameters.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CPatternedInfo
{
    friend class CPatterned;
    float x0_mass;
    float x4_speed;
    float x8_turnSpeed;
    float xc_detectionRange;
    float x10_detectionHeightRange;
    float x14_dectectionAngle;
    float x18_minAttackRange;
    float x1c_maxAttackRange;
    float x20_averageAttackTime;
    float x24_attackTimeVariation;
    float x28_leashRadius;
    float x2c_playerLeashRadius;
    float x30_playerLeashTime;
    CDamageInfo x34_damageInfo;
    float x50_damageWaitTime;
    CHealthInfo x54_healthInfo;
    CDamageVulnerability x5c_damageVulnerability;
    float xc4_halfExtent;
    float xc8_height;
    zeus::CVector3f xcc_bodyOrigin;
    float xd8_stepUpHeight;
    float xdc_;
    float xe0_;
    float xe4_;
    u16 xe8_deathSfx;
    CAnimationParameters xec_animParams;
    bool xf8_active;
    ResId xfc_stateMachineId;
    float x100_;
    float x104_;
    float x108_;

    u32 x10c_particle1Frames;
    zeus::CVector3f x110_particle1Scale;
    ResId x11c_particle1;

    u32 x120_particle2Frames;
    zeus::CVector3f x124_particle2Scale;
    ResId x130_particle2 = -1;

    u16 x134_iceShatterSfx = -1;

public:
    CPatternedInfo(CInputStream& in, u32 pcount);
    static std::pair<bool, u32> HasCorrectParameterCount(CInputStream& in);

    CAnimationParameters& GetAnimationParameters() { return xec_animParams; }
    const CAnimationParameters& GetAnimationParameters() const { return xec_animParams; }
};
}

#endif // __URDE_CPATTERNEDINFO_HPP__
