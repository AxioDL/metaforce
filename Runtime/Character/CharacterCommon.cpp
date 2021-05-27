#include "CharacterCommon.hpp"

using namespace std::literals;

namespace metaforce::pas {
std::string_view AnimationStateToStr(EAnimationState state) {
  switch (state) {
  case EAnimationState::Invalid:
    return "Invalid"sv;
  case EAnimationState::Fall:
    return "Fall"sv;
  case EAnimationState::Getup:
    return "Getup"sv;
  case EAnimationState::LieOnGround:
    return "LieOnGround"sv;
  case EAnimationState::Step:
    return "Step"sv;
  case EAnimationState::Death:
    return "Death"sv;
  case EAnimationState::Locomotion:
    return "Locomotion"sv;
  case EAnimationState::KnockBack:
    return "KnockBack"sv;
  case EAnimationState::MeleeAttack:
    return "MeleeAttack"sv;
  case EAnimationState::Turn:
    return "Turn"sv;
  case EAnimationState::LoopAttack:
    return "LoopAttack"sv;
  case EAnimationState::LoopReaction:
    return "LoopReaction"sv;
  case EAnimationState::GroundHit:
    return "GroundHit"sv;
  case EAnimationState::Generate:
    return "Generate"sv;
  case EAnimationState::Jump:
    return "Jump"sv;
  case EAnimationState::Hurled:
    return "Hurled"sv;
  case EAnimationState::Slide:
    return "Slide"sv;
  case EAnimationState::Taunt:
    return "Taunt"sv;
  case EAnimationState::Scripted:
    return "Scripted"sv;
  case EAnimationState::ProjectileAttack:
    return "ProjectileAttack"sv;
  case EAnimationState::Cover:
    return "Cover"sv;
  case EAnimationState::WallHang:
    return "WallHang"sv;
  case EAnimationState::AdditiveIdle:
    return "AdditiveIdle"sv;
  case EAnimationState::AdditiveAim:
    return "AdditiveAim"sv;
  case EAnimationState::AdditiveFlinch:
    return "AdditiveFlinch"sv;
  case EAnimationState::AdditiveReaction:
    return "AdditiveReaction"sv;
  default:
    return "[unknown]";
  }
}
} // namespace metaforce::pas
