#include "Runtime/World/CScriptColorModulate.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Graphics/CModel.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptColorModulate::CScriptColorModulate(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                           const zeus::CColor& colorA, const zeus::CColor& colorB, EBlendMode blendMode,
                                           float timeA2B, float timeB2A, bool doReverse, bool resetTargetWhenDone,
                                           bool depthCompare, bool depthUpdate, bool depthBackwards, bool active)
: CEntity(uid, info, active, name)
, x40_colorA(colorA)
, x44_colorB(colorB)
, x48_blendMode(blendMode)
, x4c_timeA2B(timeA2B)
, x50_timeB2A(timeB2A)
, x54_24_doReverse(doReverse)
, x54_25_resetTargetWhenDone(resetTargetWhenDone)
, x54_26_depthCompare(depthCompare)
, x54_27_depthUpdate(depthUpdate)
, x54_28_depthBackwards(depthBackwards) {}

void CScriptColorModulate::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptColorModulate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& mgr) {
  CEntity::AcceptScriptMsg(msg, objId, mgr);

  if (!GetActive())
    return;

  switch (msg) {
  case EScriptObjectMessage::Increment:
    if (x54_29_reversing) {
      x38_fadeState = x38_fadeState == EFadeState::A2B ? EFadeState::B2A : EFadeState::A2B;
      x54_29_reversing = false;
      return;
    } else if (x54_30_enable) {
      if (x38_fadeState == EFadeState::A2B)
        x3c_curTime = 0.f;
      else
        x3c_curTime = x4c_timeA2B - x4c_timeA2B * (x3c_curTime / x50_timeB2A);
    } else
      SetTargetFlags(mgr, CalculateFlags(x40_colorA));

    x54_30_enable = true;
    x38_fadeState = EFadeState::A2B;
    break;
  case EScriptObjectMessage::Decrement:
    if (x54_29_reversing) {
      x38_fadeState = x38_fadeState == EFadeState::A2B ? EFadeState::B2A : EFadeState::A2B;
      x54_29_reversing = false;
      return;
    } else if (x54_30_enable) {
      if (x38_fadeState == EFadeState::A2B)
        x3c_curTime = 0.f;
      else
        x3c_curTime = x50_timeB2A - x50_timeB2A * (x3c_curTime / x4c_timeA2B);
    } else
      SetTargetFlags(mgr, CalculateFlags(x44_colorB));

    x54_30_enable = true;
    x38_fadeState = EFadeState::B2A;
    break;
  default:
    break;
  }
}

void CScriptColorModulate::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !x54_30_enable)
    return;

  x3c_curTime += dt;
  if (x38_fadeState == EFadeState::A2B) {
    float t;
    if (zeus::close_enough(x4c_timeA2B, 0.f))
      t = 1.f;
    else {
      t = x3c_curTime / x4c_timeA2B;
      if (t >= 1.f)
        t = 1.f;
    }

    zeus::CColor lerpedCol = zeus::CColor::lerp(x40_colorA, x44_colorB, t);
    CModelFlags flags = CalculateFlags(lerpedCol);
    SetTargetFlags(mgr, flags);

    if (x3c_curTime <= x4c_timeA2B)
      return;

    End(mgr);
  } else if (x38_fadeState == EFadeState::B2A) {
    float t;
    if (zeus::close_enough(x50_timeB2A, 0.f))
      t = 1.f;
    else {
      t = x3c_curTime / x50_timeB2A;
      if (t >= 1.f)
        t = 1.f;
    }

    zeus::CColor lerpedCol = zeus::CColor::lerp(x44_colorB, x40_colorA, t);
    CModelFlags flags = CalculateFlags(lerpedCol);
    SetTargetFlags(mgr, flags);

    if (x3c_curTime <= x50_timeB2A)
      return;

    End(mgr);
  }
}

CModelFlags CScriptColorModulate::CalculateFlags(const zeus::CColor& col) const {
  CModelFlags ret;
  if (x54_28_depthBackwards) {
    if (x48_blendMode == EBlendMode::Alpha) {
      ret.x0_blendMode = 5;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1 | 3 | 8;
      ret.x4_color = col;
    } else if (x48_blendMode == EBlendMode::Additive) {
      ret.x0_blendMode = 7;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1 | 0x8;
      ret.x4_color = col;
    } else if (x48_blendMode == EBlendMode::Additive2) {
      ret.x0_blendMode = 8;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1 | 0x8;
      ret.x4_color = col;
    } else if (x48_blendMode == EBlendMode::Opaque) {
      ret.x0_blendMode = 1;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1 | 0x8;
      ret.x4_color = col;
    } else if (x48_blendMode == EBlendMode::OpaqueAdd) {
      ret.x0_blendMode = 2;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1 | 0x8;
      ret.addColor = col;
    } else {
      ret.x2_flags = 3;
      ret.x4_color = zeus::skWhite;
    }
  } else {
    if (x48_blendMode == EBlendMode::Alpha) {
      if (col == zeus::skWhite) {
        ret.x0_blendMode = 3;
        ret.x1_matSetIdx = 0;
        ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
        ret.x4_color = zeus::skWhite;
      } else {
        ret.x0_blendMode = 5;
        ret.x1_matSetIdx = 0;
        ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
        ret.x4_color = col;
      }
    } else if (x48_blendMode == EBlendMode::Additive) {
      ret.x0_blendMode = 7;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
      ret.x4_color = col;
    } else if (x48_blendMode == EBlendMode::Additive2) {
      ret.x0_blendMode = 8;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
      ret.x4_color = col;
    } else if (x48_blendMode == EBlendMode::Opaque) {
      if (col == zeus::skWhite) {
        ret.x0_blendMode = 3;
        ret.x1_matSetIdx = 0;
        ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
        ret.x4_color = zeus::skWhite;
      } else {
        ret.x0_blendMode = 1;
        ret.x1_matSetIdx = 0;
        ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
        ret.x4_color = col;
      }
    } else if (x48_blendMode == EBlendMode::OpaqueAdd) {
      ret.x0_blendMode = 2;
      ret.x1_matSetIdx = 0;
      ret.x2_flags = x54_26_depthCompare << 0 | x54_27_depthUpdate << 1;
      ret.addColor = col;
    } else {
      ret.x2_flags = 3;
      ret.x4_color = zeus::skWhite;
    }
  }
  return ret;
}

void CScriptColorModulate::SetTargetFlags(CStateManager& stateMgr, const CModelFlags& flags) {
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
      continue;

    auto search = stateMgr.GetIdListForScript(conn.x8_objId);
    for (auto it = search.first; it != search.second; ++it) {
      CEntity* ent = stateMgr.ObjectById(it->second);
      if (CActor* act = TCastToPtr<CActor>(ent))
        act->SetDrawFlags(flags);
    }
  }

  if (x34_parent != kInvalidUniqueId) {
    CEntity* ent = stateMgr.ObjectById(x34_parent);
    if (CActor* act = TCastToPtr<CActor>(ent))
      act->SetDrawFlags(flags);
  }
}

TUniqueId CScriptColorModulate::FadeOutHelper(CStateManager& mgr, TUniqueId parent, float fadeTime) {
  TAreaId aId = mgr.GetNextAreaId();
  if (const CEntity* ent = mgr.GetObjectById(parent))
    aId = ent->GetAreaIdAlways();

  TUniqueId ret = mgr.AllocateUniqueId();
  CScriptColorModulate* colMod = new CScriptColorModulate(
      ret, "", CEntityInfo(aId, CEntity::NullConnectionList), zeus::CColor(1.f, 1.f, 1.f, 1.f),
      zeus::CColor(1.f, 1.f, 1.f, 0.f), EBlendMode::Alpha, fadeTime, 0.f, false, false, true, true, false, true);
  mgr.AddObject(colMod);
  colMod->x34_parent = parent;
  colMod->x54_30_enable = true;
  colMod->x54_31_dieOnEnd = true;
  colMod->x55_24_isFadeOutHelper = true;

  colMod->Think(0.f, mgr);
  return ret;
}

TUniqueId CScriptColorModulate::FadeInHelper(CStateManager& mgr, TUniqueId parent, float fadeTime) {
  TAreaId aId = mgr.GetNextAreaId();
  if (const CEntity* ent = mgr.GetObjectById(parent))
    aId = ent->GetAreaIdAlways();

  TUniqueId ret = mgr.AllocateUniqueId();
  CScriptColorModulate* colMod = new CScriptColorModulate(
      ret, "", CEntityInfo(aId, CEntity::NullConnectionList), zeus::CColor(1.f, 1.f, 1.f, 0.f),
      zeus::CColor(1.f, 1.f, 1.f, 1.f), EBlendMode::Alpha, fadeTime, 0.f, false, false, true, true, false, true);
  mgr.AddObject(colMod);
  colMod->x34_parent = parent;
  colMod->x54_30_enable = true;
  colMod->x54_31_dieOnEnd = true;

  colMod->Think(0.f, mgr);
  return ret;
}

void CScriptColorModulate::End(CStateManager& stateMgr) {
  x3c_curTime = 0.f;
  if (x54_24_doReverse && !x54_29_reversing) {
    x54_29_reversing = true;
    x38_fadeState = x38_fadeState == EFadeState::A2B ? EFadeState::B2A : EFadeState::A2B;
    return;
  }

  x54_30_enable = false;
  x54_29_reversing = false;
  if (x54_25_resetTargetWhenDone)
    SetTargetFlags(stateMgr, CModelFlags(0, 0, 3, zeus::skWhite));

  if (x55_24_isFadeOutHelper)
    stateMgr.SendScriptMsgAlways(x34_parent, x8_uid, EScriptObjectMessage::Deactivate);

  CEntity::SendScriptMsgs(EScriptObjectState::MaxReached, stateMgr, EScriptObjectMessage::None);

  if (x54_31_dieOnEnd)
    stateMgr.FreeScriptObject(GetUniqueId());
}
} // namespace urde
