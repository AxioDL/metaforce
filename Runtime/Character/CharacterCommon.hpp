#pragma once

namespace urde {
namespace pas {
enum class ELocomotionType {
  Invalid = -1,
  Crouch = 0,
  Relaxed = 1,
  Lurk = 2,
  Combat = 3,
  Internal4 = 4,
  Internal5 = 5,
  Internal6 = 6,
  Internal7 = 7,
  Internal8 = 8,
  Internal9 = 9,
  Internal10 = 10,
  Internal11 = 11,
  Internal12 = 12,
  Internal13 = 13,
  Internal14 = 14
};

enum class ELocomotionAnim { Invalid = -1, Idle, Walk, Run, BackUp, StrafeLeft, StrafeRight, StrafeUp, StrafeDown };

enum class EAnimationState {
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

enum class EHurledState {
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

enum class EFallState { Invalid = -1, Zero };

enum class EReactionType { Invalid = -1, Zero, One, Two, Three };

enum class EAdditiveReactionType { Invalid = -1, Electrocution, One, Two, IceBreakout };

enum class EJumpType { Normal, One, Ambush };

enum class EJumpState { Invalid = -1, IntoJump, AmbushJump, Loop, OutOfJump, WallBounceLeft, WallBounceRight };

enum class EStepDirection { Invalid = -1, Forward = 0, Backward = 1, Left = 2, Right = 3, Up = 4, Down = 5 };

enum class EStepType { Normal = 0, Dodge = 1, BreakDodge = 2, RollDodge = 3 };

enum class ESeverity { Invalid = -1, Zero = 0, One = 1, Two = 2, Three, Four, Seven = 7, Eight = 8 };

enum class EGetupType { Invalid = -1, Zero = 0, One = 1, Two = 2 };

enum class ELoopState { Invalid = -1, Begin, Loop, End };

enum class ELoopAttackType { Invalid = -1 };

enum class EGenerateType { Invalid = -1, Zero, One, Two, Three, Four };

enum class ESlideType { Invalid = -1, Zero = 0 };

enum class ETauntType { Invalid = -1, Zero, One, Two };

enum class ECoverState { Invalid = -1, IntoCover, Cover, Lean, OutOfCover };

enum class ECoverDirection { Invalid = -1, Left, Right };

enum class ETurnDirection { Invalid = -1, Right, Left };

enum class EWallHangState {
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
} // namespace pas

enum class EBodyType { Invalid, BiPedal, Restricted, Flyer, Pitchable, RestrictedFlyer, WallWalker, NewFlyer };

enum class EBodyStateCmd {
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
  MaintainVelocity,
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

} // namespace urde
