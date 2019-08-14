#include "CAdditiveBodyState.hpp"
#include "CBodyController.hpp"
#include "Character/CPASDatabase.hpp"
#include "CStateManager.hpp"
#include "CAnimTreeNode.hpp"
#include "CPASAnimParmData.hpp"

namespace urde {

void CABSAim::Start(CBodyController& bc, CStateManager& mgr) {
  // const CBCAdditiveAimCmd* cmd =
  //    static_cast<const CBCAdditiveAimCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveAim));
  const CPASAnimState* aimState = bc.GetPASDatabase().GetAnimState(22);

  // Left, Right, Up, Down
  for (int i = 0; i < 4; ++i) {
    CPASAnimParmData parms(22, CPASAnimParm::FromEnum(i));
    std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
    x8_anims[i] = best.second;
    x18_angles[i] = zeus::degToRad(aimState->GetAnimParmData(x8_anims[i], 1).GetReal32Value());
  }

  const CAnimData& animData = *bc.GetOwner().GetModelData()->GetAnimationData();
  x28_hWeight = -animData.GetAdditiveAnimationWeight(x8_anims[0]);
  x28_hWeight += animData.GetAdditiveAnimationWeight(x8_anims[1]);
  x30_vWeight = -animData.GetAdditiveAnimationWeight(x8_anims[2]);
  x30_vWeight += animData.GetAdditiveAnimationWeight(x8_anims[3]);

  x4_needsIdle = false;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveIdle))
    x4_needsIdle = true;
}

pas::EAnimationState CABSAim::GetBodyStateTransition(float dt, CBodyController& bc) const {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveReaction))
    return pas::EAnimationState::AdditiveReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveFlinch))
    return pas::EAnimationState::AdditiveFlinch;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveIdle) || x4_needsIdle)
    return pas::EAnimationState::AdditiveIdle;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CABSAim::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    const zeus::CVector3f& target = bc.GetCommandMgr().GetAdditiveTargetVector();
    if (target.canBeNormalized()) {
      CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();

      float hAngle = zeus::clamp(-x18_angles[0], std::atan2(target.x(), target.y()), x18_angles[1]);
      hAngle *= 0.63661975f;
      hAngle = zeus::clamp(-3.f, (hAngle - x28_hWeight) * 0.25f / dt, 3.f);
      x2c_hWeightVel += dt * zeus::clamp(-10.f, (hAngle - x2c_hWeightVel) / dt, 10.f);

      float hypotenuse = std::sqrt(target.y() * target.y() + target.x() * target.x());
      float vAngle = zeus::clamp(-x18_angles[3], std::atan2(target.z(), hypotenuse), x18_angles[2]);
      vAngle *= 0.63661975f;
      vAngle = zeus::clamp(-3.f, (vAngle - x30_vWeight) * 0.25f / dt, 3.f);
      x34_vWeightVel += dt * zeus::clamp(-10.f, (vAngle - x34_vWeightVel) / dt, 10.f);

      float newHWeight = dt * x2c_hWeightVel + x28_hWeight;
      if (newHWeight != x28_hWeight) {
        if (std::fabs(x28_hWeight) > 0.f && x28_hWeight * newHWeight <= 0.f)
          animData.DelAdditiveAnimation(x8_anims[x28_hWeight < 0.f ? 0 : 1]);
        float absWeight = std::fabs(newHWeight);
        if (absWeight > 0.f)
          animData.AddAdditiveAnimation(x8_anims[newHWeight < 0.f ? 0 : 1], absWeight, false, false);
      }

      float newVWeight = dt * x34_vWeightVel + x30_vWeight;
      if (newVWeight != x30_vWeight) {
        if (std::fabs(x30_vWeight) > 0.f && x30_vWeight * newVWeight <= 0.f)
          animData.DelAdditiveAnimation(x8_anims[x30_vWeight > 0.f ? 2 : 3]);
        float absWeight = std::fabs(newVWeight);
        if (absWeight > 0.f)
          animData.AddAdditiveAnimation(x8_anims[newVWeight > 0.f ? 2 : 3], absWeight, false, false);
      }

      x28_hWeight = newHWeight;
      x30_vWeight = newVWeight;
    }
  }
  return st;
}

void CABSAim::Shutdown(CBodyController& bc) {
  CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();

  if (x28_hWeight != 0.f)
    animData.DelAdditiveAnimation(x8_anims[x28_hWeight < 0.f ? 0 : 1]);
  if (x30_vWeight != 0.f)
    animData.DelAdditiveAnimation(x8_anims[x30_vWeight > 0.f ? 2 : 3]);
}

void CABSFlinch::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveFlinchCmd* cmd =
      static_cast<const CBCAdditiveFlinchCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveFlinch));
  x4_weight = cmd->GetWeight();

  CPASAnimParmData parms(23);
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  x8_anim = best.second;

  CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
  animData.AddAdditiveAnimation(x8_anim, x4_weight, false, true);
}

pas::EAnimationState CABSFlinch::GetBodyStateTransition(float dt, CBodyController& bc) const {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveReaction))
    return pas::EAnimationState::AdditiveReaction;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CABSFlinch::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    CCharAnimTime rem = animData.GetAdditiveAnimationTree(x8_anim)->VGetTimeRemaining();
    if (std::fabs(rem.GetSeconds()) < 0.00001f)
      return pas::EAnimationState::AdditiveIdle;
  }
  return st;
}

pas::EAnimationState CABSIdle::GetBodyStateTransition(float dt, CBodyController& bc) const {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveReaction))
    return pas::EAnimationState::AdditiveReaction;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveFlinch))
    return pas::EAnimationState::AdditiveFlinch;
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveAim))
    return pas::EAnimationState::AdditiveAim;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CABSIdle::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  return GetBodyStateTransition(dt, bc);
}

void CABSReaction::Start(CBodyController& bc, CStateManager& mgr) {
  const CBCAdditiveReactionCmd* cmd =
      static_cast<const CBCAdditiveReactionCmd*>(bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveReaction));
  x4_weight = cmd->GetWeight();
  xc_type = cmd->GetType();
  x10_active = cmd->GetIsActive();

  CPASAnimParmData parms(24, CPASAnimParm::FromEnum(s32(xc_type)));
  std::pair<float, s32> best = bc.GetPASDatabase().FindBestAnimation(parms, *mgr.GetActiveRandom(), -1);
  x8_anim = best.second;

  if (x8_anim != -1) {
    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    animData.AddAdditiveAnimation(x8_anim, x4_weight, x10_active, false);
  }
}

pas::EAnimationState CABSReaction::GetBodyStateTransition(float dt, CBodyController& bc) const {
  if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::AdditiveReaction) && xc_type == pas::EAdditiveReactionType::IceBreakout)
    return pas::EAnimationState::AdditiveReaction;
  return pas::EAnimationState::Invalid;
}

pas::EAnimationState CABSReaction::UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) {
  const pas::EAnimationState st = GetBodyStateTransition(dt, bc);
  if (st == pas::EAnimationState::Invalid) {
    if (x8_anim == -1)
      return pas::EAnimationState::AdditiveIdle;

    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    if (x10_active) {
      if (bc.GetCommandMgr().GetCmd(EBodyStateCmd::StopReaction)) {
        StopAnimation(bc);
        bc.GetOwner().RemoveEmitter();
        return pas::EAnimationState::AdditiveIdle;
      }
    } else {
      if (animData.IsAdditiveAnimationAdded(x8_anim)) {
        CCharAnimTime rem = animData.GetAdditiveAnimationTree(x8_anim)->VGetTimeRemaining();
        if (std::fabs(rem.GetSeconds()) < 0.00001f) {
          StopAnimation(bc);
          return pas::EAnimationState::AdditiveIdle;
        }
      } else {
        return pas::EAnimationState::AdditiveIdle;
      }
    }
  }
  return st;
}

void CABSReaction::StopAnimation(CBodyController& bc) {
  if (x8_anim != -1) {
    CAnimData& animData = *bc.GetOwner().ModelData()->AnimationData();
    animData.DelAdditiveAnimation(x8_anim);
    x8_anim = -1;
  }
}

} // namespace urde
