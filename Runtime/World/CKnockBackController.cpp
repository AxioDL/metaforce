#include "CKnockBackController.hpp"
#include "CPatterned.hpp"
#include "CStateManager.hpp"
#include "Character/CPASAnimParmData.hpp"

namespace urde
{

static const CKnockBackController::KnockBackParms KnockBackParmsTable[3][19][4] =
{
    {
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 1.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 5.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 5.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 8.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 4.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Six, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
    },
    {
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 1.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 5.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 5.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 8.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 8.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 4.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 4.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Eight, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Eight, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Eight, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Eight, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Nine, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Nine, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Four, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
    },
    {
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 1.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 2.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Two, 4.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Two, 4.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 4.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 4.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::One, 2.500000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Three, 6.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Eight, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Three, 6.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::KnockBack, EKnockBackAnimationCondition::Seven, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::None, EKnockBackAnimationCondition::Zero, 0.000000f, 0.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Nine, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Nine, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
        {
            {EKnockBackAnimationState::Flinch, EKnockBackAnimationCondition::Zero, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Four, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Hurled, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
            {EKnockBackAnimationState::Fall, EKnockBackAnimationCondition::Seven, 0.000000f, -1.000000f},
        },
    },
};

CKnockBackController::CKnockBackController(EKnockBackVariant variant)
: x0_variant(variant)
{
    x81_24_autoResetImpulse = true;
    x81_25_ = true;
    x81_27_ = true;
    x81_28_ = true;
    x81_29_ = true;
    x81_30_ = true;
    x81_31_ = true;
    x82_24_ = true;
    for (int i = 0; i < 4; ++i)
    {
        x24_.push_back(std::make_pair(0.f, FLT_MAX));
        x80_availableStates.set(i);
    }
}

static const float ImpulseDurationTable[] = { 0.1f, 0.3f };

void CKnockBackController::ApplyImpulse(float dt, CPatterned& parent)
{
    x60_impulseRemTime = std::max(0.f, x60_impulseRemTime - dt);
    if (!parent.GetMaterialList().HasMaterial(EMaterialTypes::Immovable) && x60_impulseRemTime > 0.f)
    {
        float remFac = 1.f;
        if (x20_impulseDurationIdx == 1)
            remFac = x60_impulseRemTime / ImpulseDurationTable[x20_impulseDurationIdx];
        parent.ApplyImpulseWR(
            parent.GetMoveToORImpulseWR(
                parent.GetTransform().transposeRotate(x50_impulseDir *
                (remFac * x5c_impulseMag * dt / ImpulseDurationTable[x20_impulseDurationIdx])), dt),
            zeus::CAxisAngle::sIdentity);
    }
}

bool CKnockBackController::TickDeferredTimer(float dt)
{
    x68_deferRemTime -= dt;
    if (x14_deferWeaponType != EWeaponType::None)
        return x68_deferRemTime <= 0.f;
    return false;
}

EKnockBackCharacterState CKnockBackController::GetKnockBackCharacterState(CPatterned& parent)
{
    if (parent.BodyController()->IsFrozen())
        return parent.IsAlive() ? EKnockBackCharacterState::FrozenAlive : EKnockBackCharacterState::FrozenDead;
    return parent.IsAlive() ? EKnockBackCharacterState::Alive : EKnockBackCharacterState::Dead;
}

void CKnockBackController::ValidateState(CPatterned& parent)
{
    if (x4_activeParms.x0_animState < x18_minAnimState)
        x4_activeParms.x0_animState = x18_minAnimState;
    else if (x4_activeParms.x0_animState > x1c_maxAnimState)
        x4_activeParms.x0_animState = x1c_maxAnimState;

    EKnockBackAnimationState useState = EKnockBackAnimationState::Invalid;
    if (parent.IsAlive())
    {
        if (parent.BodyController()->HasBodyState(pas::EAnimationState::Hurled) &&
            x80_availableStates.test(3) && x4_activeParms.x0_animState >= EKnockBackAnimationState::Hurled)
        {
            useState = EKnockBackAnimationState::Hurled;
        }
        else if (parent.BodyController()->HasBodyState(pas::EAnimationState::KnockBack) &&
                 x80_availableStates.test(2) && x4_activeParms.x0_animState >= EKnockBackAnimationState::KnockBack)
        {
            useState = EKnockBackAnimationState::KnockBack;
        }
        else if (parent.BodyController()->HasBodyState(pas::EAnimationState::AdditiveFlinch) &&
                 x80_availableStates.test(1) && x4_activeParms.x0_animState >= EKnockBackAnimationState::Flinch)
        {
            useState = EKnockBackAnimationState::Flinch;
        }
    }
    else
    {
        if (parent.BodyController()->HasBodyState(pas::EAnimationState::Fall) && x80_availableStates.test(4) &&
            (x4_activeParms.x0_animState >= EKnockBackAnimationState::Fall ||
            (!parent.BodyController()->HasBodyState(pas::EAnimationState::Hurled)
            && x4_activeParms.x0_animState >= EKnockBackAnimationState::Hurled)))
        {
            useState = EKnockBackAnimationState::Fall;
        }
        else if (parent.BodyController()->HasBodyState(pas::EAnimationState::Hurled) &&
                 x80_availableStates.test(3) && x4_activeParms.x0_animState >= EKnockBackAnimationState::Hurled)
        {
            useState = EKnockBackAnimationState::Hurled;
        }
    }

    x4_activeParms.x0_animState = (useState != EKnockBackAnimationState::Invalid) ?
        useState : EKnockBackAnimationState::None;

    bool r4 = false;
    switch (x4_activeParms.x4_animCondition)
    {
    case EKnockBackAnimationCondition::One:
        r4 = !x81_25_;
        break;
    case EKnockBackAnimationCondition::Two:
        r4 = !x81_26_;
        break;
    case EKnockBackAnimationCondition::Three:
        r4 = !x81_27_;
        break;
    case EKnockBackAnimationCondition::Six:
        r4 = !x81_29_;
        break;
    case EKnockBackAnimationCondition::Seven:
        r4 = !x81_29_;
        break;
    case EKnockBackAnimationCondition::Eight:
        r4 = !x81_28_;
        break;
    case EKnockBackAnimationCondition::Nine:
        r4 = !x81_30_;
        break;
    default:
        break;
    }

    if (r4)
    {
        x4_activeParms.x4_animCondition = EKnockBackAnimationCondition::Zero;
        x4_activeParms.x8_ = 0.f;
    }
}

float CKnockBackController::CalculateExtraHurlVelocity(CStateManager& mgr, float magnitude, float kbResistance)
{
    if (magnitude > kbResistance)
        return (1.1f - 0.2f * mgr.GetActiveRandom()->Float()) * 2.f * (magnitude - kbResistance);
    return 0.f;
}

void CKnockBackController::DoKnockBackAnimation(const zeus::CVector3f& backVec, CStateManager& mgr,
                                                CPatterned& parent, float magnitude)
{
    switch (x4_activeParms.x0_animState)
    {
    case EKnockBackAnimationState::Hurled:
    {
        float hurlVel = 5.f;
        if (CHealthInfo* hInfo = parent.HealthInfo(mgr))
            hurlVel += CalculateExtraHurlVelocity(mgr, magnitude, hInfo->GetKnockbackResistance());
        hurlVel = std::sqrt(parent.GetGravityConstant() * 0.5f * hurlVel);
        zeus::CVector3f backUpVec = backVec + backVec.magnitude() * zeus::CVector3f::skUp;
        if (backUpVec.canBeNormalized())
        {
            parent.BodyController()->GetCommandMgr().DeliverCmd(
                CBCHurledCmd(-backVec, backUpVec.normalized() * hurlVel));
            parent.SetMomentumWR({0.f, 0.f, parent.GetGravityConstant() * -parent.GetMass()});
        }
        break;
    }
    case EKnockBackAnimationState::Fall:
    {
        parent.BodyController()->GetCommandMgr().DeliverCmd(CBCKnockDownCmd(-backVec, x7c_severity));
        break;
    }
    case EKnockBackAnimationState::KnockBack:
    {
        parent.BodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(-backVec, x7c_severity));
        break;
    }
    case EKnockBackAnimationState::Flinch:
    {
        std::pair<float, s32> bestAnim =
        parent.BodyController()->GetPASDatabase().FindBestAnimation(
            CPASAnimParmData(23), *mgr.GetActiveRandom(), -1);
        if (bestAnim.first > 0.f)
        {
            parent.ModelData()->AnimationData()->AddAdditiveAnimation(bestAnim.second, 1.f, false, true);
            x64_flinchRemTime = std::max(
                parent.ModelData()->AnimationData()->GetAnimationDuration(bestAnim.second), x64_flinchRemTime);
        }
        break;
    }
    default:
        break;
    }
}

void CKnockBackController::ResetKnockBackImpulse(CPatterned& parent, const zeus::CVector3f& backVec, float magnitude)
{
    if (x81_24_autoResetImpulse && x4_activeParms.x0_animState == EKnockBackAnimationState::KnockBack &&
        x4_activeParms.x4_animCondition != EKnockBackAnimationCondition::One)
    {
        x50_impulseDir = backVec.canBeNormalized() ? backVec.normalized() : -parent.GetTransform().basis[1];
        if (x60_impulseRemTime <= 0.f)
            x5c_impulseMag = magnitude;
        else
            x5c_impulseMag += magnitude * (1.f - x60_impulseRemTime / ImpulseDurationTable[x20_impulseDurationIdx]);
        x60_impulseRemTime = ImpulseDurationTable[x20_impulseDurationIdx];
    }
}

void CKnockBackController::DoDeferredKnockBack(CStateManager& mgr, CPatterned& parent)
{
    if (x14_deferWeaponType == EWeaponType::Wave)
    {
        x4_activeParms = KnockBackParmsTable[int(x0_variant)][int(EKnockBackWeaponType::WaveExtra)]
                                            [int(GetKnockBackCharacterState(parent))];
        ValidateState(parent);
        if (CHealthInfo* hInfo = parent.HealthInfo(mgr))
        {
            zeus::CVector3f backVec = -parent.GetTransform().basis[1];
            DoKnockBackAnimation(backVec, mgr, parent, 10.f);
            ResetKnockBackImpulse(parent, backVec, 2.f);
            x82_25_inDeferredKnockBack = true;
            parent.KnockBack(backVec, mgr,
                CDamageInfo({x14_deferWeaponType, false, true, false}, 0.f, 0.f, 10.f),
                EKnockBackType::One, x82_25_inDeferredKnockBack, 10.f);
            x82_25_inDeferredKnockBack = false;
        }
    }
    x68_deferRemTime = 0.f;
    x4_activeParms = KnockBackParms{};
    x14_deferWeaponType = EWeaponType::None;
}

void CKnockBackController::sub80233d40(int i, float f1, float f2)
{
    if (i < 0 || i > 4)
        return;
    x24_[i] = std::make_pair(f1, f2);
}

void CKnockBackController::SetAutoResetImpulse(bool b)
{
    x81_24_autoResetImpulse = b;
    if (!b)
    {
        x5c_impulseMag = 0.f;
        x60_impulseRemTime = 0.f;
    }
}

void CKnockBackController::Update(float dt, CStateManager& mgr, CPatterned& parent)
{
    ApplyImpulse(dt, parent);
    x64_flinchRemTime -= dt;
    if (TickDeferredTimer(dt))
        DoDeferredKnockBack(mgr, parent);
    if (x82_26_ && parent.BodyController()->IsElectrocuting())
        parent.BodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::Locomotion));
}

EKnockBackWeaponType CKnockBackController::GetKnockBackWeaponType(const CDamageInfo& info, EWeaponType wType,
                                                                  EKnockBackType type)
{
    int stacking = 0;
    if (info.GetWeaponMode().IsCharged())
        stacking = 1;
    else if (info.GetWeaponMode().IsComboed())
        stacking = 2;
    if (wType > EWeaponType::Phazon)
        return EKnockBackWeaponType::Invalid;
    switch (wType)
    {
    case EWeaponType::Power:
        return EKnockBackWeaponType(type != EKnockBackType::Zero ? stacking : stacking + 1);
    case EWeaponType::Ice:
        return EKnockBackWeaponType(type != EKnockBackType::Zero ? stacking + 8 : stacking + 9);
    case EWeaponType::Wave:
        return EKnockBackWeaponType(stacking + 4);
    case EWeaponType::Plasma:
        return EKnockBackWeaponType(stacking + 12);
    case EWeaponType::Bomb:
        return EKnockBackWeaponType::Bomb;
    case EWeaponType::PowerBomb:
        return EKnockBackWeaponType::PowerBomb;
    case EWeaponType::Missile:
        return EKnockBackWeaponType::Missile;
    case EWeaponType::Phazon:
        return EKnockBackWeaponType::Phazon;
    default:
        return EKnockBackWeaponType::Invalid;
    }
}

void CKnockBackController::SelectDamageState(CPatterned& parent, const CDamageInfo& info,
                                             EWeaponType wType, EKnockBackType type)
{
    EKnockBackWeaponType weaponType = GetKnockBackWeaponType(info, wType, type);
    if (weaponType != EKnockBackWeaponType::Invalid)
    {
        x4_activeParms = KnockBackParmsTable[int(x0_variant)][int(weaponType)]
                                            [int(GetKnockBackCharacterState(parent))];
        ValidateState(parent);
    }
}

void CKnockBackController::KnockBack(const zeus::CVector3f& backVec, CStateManager& mgr, CPatterned& parent,
                                     const CDamageInfo& info, EKnockBackType type, float magnitude)
{
    if (!x82_25_inDeferredKnockBack)
    {
        zeus::CVector3f vec(backVec.toVec2f());
        if (!vec.isMagnitudeSafe())
            vec = -parent.GetTransform().basis[1];
        SelectDamageState(parent, info, info.GetWeaponMode().GetType(), type);
        DoKnockBackAnimation(vec, mgr, parent, magnitude);
        ResetKnockBackImpulse(parent, vec, 2.f);
    }
}

}
