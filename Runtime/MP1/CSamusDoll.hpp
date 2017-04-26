#ifndef __URDE_CSAMUSDOLL_HPP__
#define __URDE_CSAMUSDOLL_HPP__

#include "CPlayerState.hpp"
#include "CToken.hpp"
#include "Character/CModelData.hpp"

namespace urde
{
class CDependencyGroup;
namespace MP1
{

class CSamusDoll
{
    std::vector<CToken> x0_depToks;
    zeus::CTransform x10_;
    float x40_ = 0.f;
    CPlayerState::EPlayerSuit x44_suit;
    CPlayerState::EBeamId x48_beam;
    bool x4c_intoBallComplete = false;
    bool x4d_morphball = false;
    std::experimental::optional<CModelData> xc8_suitModel1;
    rstl::reserved_vector<TCachedToken<CSkinnedModel>, 2> x118_suitModel1and2;
    std::experimental::optional<CModelData> x134_suitModelBoots;
public:
    CSamusDoll(const CDependencyGroup& suitDgrp, const CDependencyGroup& ballDgrp,
               CPlayerState::EPlayerSuit suit, CPlayerState::EBeamId beam,
               bool hasSpiderBall, bool hasGrappleBeam);
};

}
}

#endif // __URDE_CSAMUSDOLL_HPP__
