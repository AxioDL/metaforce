#include "CBodyStateInfo.hpp"
#include "World/CActor.hpp"
#include "CBodyController.hpp"

namespace urde
{

CBodyStateInfo::CBodyStateInfo(CActor& actor, EBodyType type)
{
    x34_24_changeLocoAtEndOfAnimOnly = false;
    const CPASDatabase& pasDatabase =
        actor.GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
    for (int i=0 ; i<pasDatabase.GetNumAnimStates() ; ++i)
    {
        const CPASAnimState* state = pasDatabase.GetAnimStateByIndex(i);
        std::unique_ptr<CBodyState> bs;

        switch (type)
        {
        case EBodyType::BiPedal:
            bs = SetupBiPedalBodyStates(state->GetStateId(), actor);
            break;
        case EBodyType::Restricted:
        default:
            bs = SetupRestrictedBodyStates(state->GetStateId(), actor);
            break;
        case EBodyType::Flyer:
            bs = SetupFlyerBodyStates(state->GetStateId(), actor);
            break;
        case EBodyType::Pitchable:
            bs = SetupPitchableBodyStates(state->GetStateId(), actor);
            break;
        case EBodyType::WallWalker:
            bs = SetupWallWalkerBodyStates(state->GetStateId(), actor);
            break;
        case EBodyType::NewFlyer:
            bs = SetupNewFlyerBodyStates(state->GetStateId(), actor);
            break;
        case EBodyType::RestrictedFlyer:
            bs = SetupRestrictedFlyerBodyStates(state->GetStateId(), actor);
            break;
        }

        if (bs)
            x0_stateMap[state->GetStateId()] = std::move(bs);
    }

    x1c_additiveStates.reserve(4);
    x1c_additiveStates.push_back({21, std::make_unique<CABSIdle>()});
    x1c_additiveStates.push_back({22, std::make_unique<CABSAim>()});
    x1c_additiveStates.push_back({23, std::make_unique<CABSFlinch>()});
    x1c_additiveStates.push_back({24, std::make_unique<CABSReaction>()});
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupRestrictedFlyerBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSRestrictedFlyerLocomotion>(actor);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    default:
        return {};
    }
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupNewFlyerBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSNewFlyerLocomotion>(actor);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    default:
        return {};
    }
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupWallWalkerBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSWallWalkerLocomotion>(actor);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSFlyerTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    default:
        return {};
    }
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupPitchableBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSFlyerLocomotion>(actor, true);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSFlyerTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    default:
        return {};
    }
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupFlyerBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSFlyerLocomotion>(actor, false);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSFlyerTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    default:
        return {};
    }
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupRestrictedBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSRestrictedLocomotion>(actor);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    case 19:
        return std::make_unique<CBSCover>();
    default:
        return {};
    }
}

std::unique_ptr<CBodyState> CBodyStateInfo::SetupBiPedalBodyStates(int stateId, CActor& actor)
{
    switch (stateId)
    {
    case 0:
        return std::make_unique<CBSFall>();
    case 1:
        return std::make_unique<CBSGetup>();
    case 2:
        return std::make_unique<CBSLieOnGround>(actor);
    case 3:
        return std::make_unique<CBSStep>();
    case 4:
        return std::make_unique<CBSDie>();
    case 5:
        return std::make_unique<CBSBiPedLocomotion>(actor);
    case 6:
        return std::make_unique<CBSKnockBack>();
    case 7:
        return std::make_unique<CBSAttack>();
    case 18:
        return std::make_unique<CBSProjectileAttack>();
    case 9:
        return std::make_unique<CBSLoopAttack>();
    case 8:
        return std::make_unique<CBSTurn>();
    case 10:
        return std::make_unique<CBSLoopReaction>();
    case 11:
        return std::make_unique<CBSGroundHit>();
    case 12:
        return std::make_unique<CBSGenerate>();
    case 13:
        return std::make_unique<CBSJump>();
    case 14:
        return std::make_unique<CBSHurled>();
    case 15:
        return std::make_unique<CBSSlide>();
    case 16:
        return std::make_unique<CBSTaunt>();
    case 17:
        return std::make_unique<CBSScripted>();
    case 19:
        return std::make_unique<CBSCover>();
    case 20:
        return std::make_unique<CBSWallHang>();
    default:
        return {};
    }
}

float CBodyStateInfo::GetLocomotionSpeed(pas::ELocomotionAnim anim) const
{
    auto search = x0_stateMap.find(5);
    if (search != x0_stateMap.cend() && search->second && x18_bodyController)
    {
        const CBSLocomotion& bs = static_cast<const CBSLocomotion&>(*search->second);
        return bs.GetLocomotionSpeed(x18_bodyController->GetLocomotionType(), anim);
    }
    return 0.f;
}

float CBodyStateInfo::GetMaxSpeed() const
{
    float ret = GetLocomotionSpeed(pas::ELocomotionAnim::Run);
    if (std::fabs(ret) < 0.00001f)
    {
        for (int i=0 ; i<8 ; ++i)
        {
            float tmp = GetLocomotionSpeed(pas::ELocomotionAnim(i));
            if (tmp > ret)
                ret = tmp;
        }
    }
    return ret;
}

}
