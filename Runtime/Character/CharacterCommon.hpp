#pragma once

namespace urde
{
namespace pas
{
enum class ELocomotionType
{
    Invalid = -1,
    Crouch,
    Relaxed,
    Lurk,
    Combat,
    Internal4,
    Internal5,
    Internal6,
    Internal7,
    Internal8,
    Internal9,
    Internal10,
    Internal11,
    Internal12,
    Internal13,
    Internal14
};

enum class ELocomotionAnim
{
    Invalid = -1,
    Idle,
    Walk,
    Run,
    BackUp,
    StrafeLeft,
    StrafeRight,
    StrafeUp,
    StrafeDown
};

enum class EAnimationState
{
    Invalid = -1,
    Fall,
    Getup,
    LieOnGround,
    Step,
    Death,
    Locomotion,
    KnockBack,
    MeleeAttack,
    Turn,
    LoopAttack,
    LoopReaction,
    GroundHit,
    Generate,
    Jump,
    Hurled,
    Slide,
    Taunt,
    Scripted,
    ProjectileAttack,
    Cover,
    WallHang,
    AdditiveIdle,
    AdditiveAim,
    AdditiveFlinch,
    AdditiveReaction
};

enum class EHurledState
{
    Invalid = -1,
    KnockIntoAir,
    KnockLoop,
    KnockDown,
    StrikeWall,
    StrikeWallFallLoop,
    OutOfStrikeWall,
    Six,
    Seven
};

enum class EFallState
{
    Invalid = -1,
    Zero
};

enum class EReactionType
{
    Invalid = -1,
    Zero,
    One,
    Two,
    Three
};

enum class EAdditiveReactionType
{
    Invalid = -1,
    Electrocution,
    One,
    Two,
    IceBreakout
};

enum class EJumpType
{
    Normal,
    One,
    Ambush
};

enum class EJumpState
{
    Invalid = -1,
    IntoJump,
    AmbushJump,
    Loop,
    OutOfJump,
    WallBounceLeft,
    WallBounceRight
};

enum class EStepDirection
{
    Invalid = -1,
    Forward = 0,
    Backward = 1,
    Left = 2,
    Right = 3,
    Up = 4,
    Down = 5
};

enum class EStepType
{
    Normal = 0,
    Dodge = 1
};

enum class ESeverity
{
    Invalid = -1,
    Zero = 0,
    One = 1
};

enum class EGetupType
{
    Invalid = -1
};

enum class ELoopState
{
    Invalid = -1,
    Begin,
    Loop,
    End
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

enum class ECoverState
{
    Invalid = -1,
    IntoCover,
    Cover,
    Lean,
    OutOfCover
};

enum class ECoverDirection
{
    Invalid = -1,
    Left,
    Right
};

enum class ETurnDirection
{
    Invalid = -1,
    Right,
    Left
};

enum class EWallHangState
{
    Invalid = -1,
    IntoJump,
    JumpArc,
    JumpAirLoop,
    IntoWallHang,
    WallHang,
    Five,
    OutOfWallHang,
    OutOfWallHangTurn,
    DetachJumpLoop,
    DetachOutOfJump
};
}

enum class EBodyType
{
    Invalid,
    BiPedal,
    Restricted,
    Flyer,
    Pitchable,
    RestrictedFlyer,
    WallWalker,
    NewFlyer
};

enum class EBodyStateCmd
{
    Getup,
    Step,
    Die,
    KnockDown,
    KnockBack,
    MeleeAttack,
    ProjectileAttack,
    LoopAttack,
    LoopReaction,
    LoopHitReaction,
    ExitState,
    LeanFromCover,
    NextState,
    Thirteen,
    Generate,
    Hurled,
    Jump,
    Slide,
    Taunt,
    Scripted,
    Cover,
    WallHang,
    Locomotion,
    AdditiveIdle,
    AdditiveAim,
    AdditiveFlinch,
    AdditiveReaction,
    StopReaction
};

}

