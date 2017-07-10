#ifndef __URDE_CBODYSTATEINFO_HPP__
#define __URDE_CBODYSTATEINFO_HPP__

#include "RetroTypes.hpp"
#include "CharacterCommon.hpp"
#include "CBodyState.hpp"
#include "CAdditiveBodyState.hpp"

namespace urde
{
class CActor;

class CBodyStateInfo
{
    friend class CBodyController;
    std::map<int, std::unique_ptr<CBodyState>> x0_stateMap;
    int x14_ = -1;
    CBodyController* x18_bodyController = nullptr;
    std::vector<std::pair<int, std::unique_ptr<CAdditiveBodyState>>> x1c_additiveStates;
    u32 x2c_ = 0x15;
    float x30_maxPitch = 0.f;
    bool x34_24_changeLocoAtEndOfAnimOnly;
    std::unique_ptr<CBodyState> SetupRestrictedFlyerBodyStates(int stateId, CActor& actor);
    std::unique_ptr<CBodyState> SetupNewFlyerBodyStates(int stateId, CActor& actor);
    std::unique_ptr<CBodyState> SetupWallWalkerBodyStates(int stateId, CActor& actor);
    std::unique_ptr<CBodyState> SetupPitchableBodyStates(int stateId, CActor& actor);
    std::unique_ptr<CBodyState> SetupFlyerBodyStates(int stateId, CActor& actor);
    std::unique_ptr<CBodyState> SetupRestrictedBodyStates(int stateId, CActor& actor);
    std::unique_ptr<CBodyState> SetupBiPedalBodyStates(int stateId, CActor& actor);
public:
    CBodyStateInfo(CActor& actor, EBodyType type);
    float GetLocomotionSpeed(pas::ELocomotionAnim anim) const;
    float GetMaxSpeed() const;
    float GetMaximumPitch() const { return x30_maxPitch; }
    bool GetLocoAnimChangeAtEndOfAnimOnly() const { return x34_24_changeLocoAtEndOfAnimOnly; }
    void SetLocoAnimChangeAtEndOfAnimOnly(bool s) { x34_24_changeLocoAtEndOfAnimOnly = s; }
};

}

#endif // __URDE_CBODYSTATEINFO_HPP__
