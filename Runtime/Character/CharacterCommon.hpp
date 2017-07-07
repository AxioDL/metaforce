#ifndef __URDE_PASTYPES_HPP__
#define __URDE_PASTYPES_HPP__

namespace urde
{
namespace pas
{
enum class ELocomotionType
{
};

enum class ELocomotionAnim
{
};

enum class EAnimationState
{
};

enum class EHurledType
{

};

enum class EFallState
{
};

enum class EReactionType
{
    Invalid = -1
};

enum class EAdditiveReactionType
{

};

enum class EJumpType
{
    Zero
};

enum class EStepDirection
{
    Invalid = -1,
    Up = 0,
    Down = 1,
    Left = 2,
    Right = 3
};

enum class EStepType
{
    Normal = 0,
    Dodge = 1
};

enum class ESeverity
{
    Invalid = -1
};

enum class EGetupType
{
    Invalid = -1
};

enum class ELoopAttackType
{
    Invalid = -1
};

enum class EGenerateType
{
    Invalid = -1
};

enum class ESlideType
{
    Invalid = -1
};

enum class ETauntType
{
    Invalid = -1
};

enum class ECoverDirection
{
    Invalid = -1
};
}

enum class EBodyType
{
    Zero,
    One,
    Two,
    Three
};

enum class EBodyStateCmd
{
    Getup,
    Step,
    Two,
    KnockDown,
    KnockBack,
    MeleeAttack,
    ProjectileAttack,
    LoopAttack,
    LoopReaction,
    LoopHitReaction,
    Ten,
    Eleven,
    Twelve,
    Thirteen,
    Generate,
    Hurled,
    Jump,
    Slide,
    Taunt,
    Scripted,
    Cover,
    WallHang,
    TwentyTwo,
    TwentyThree,
    AdditiveAim,
    AdditiveFlinch,
    AdditiveReaction,
    TwentySeven
};

}

#endif // __URDE_PASTYPES_HPP__
