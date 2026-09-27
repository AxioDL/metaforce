#include "MetroidPrime/ScriptObjects/CScriptColorModulate.hpp"

#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/CActor.hpp"
#include "rstl/math.hpp"

CScriptColorModulate::CScriptColorModulate(const TUniqueId uid, const rstl::string& name,
                                           const CEntityInfo& info, const CColor& colorA,
                                           const CColor& colorB, const EBlendMode blendMode,
                                           const float timeA2B, const float timeB2A,
                                           const bool doReverse, const bool resetTargetWhenDone,
                                           const bool depthCompare, const bool depthUpdate,
                                           const bool depthBackwards, const bool active)
: CEntity(uid, info, active, name)
, mParent(kInvalidUniqueId)
, mFadeState(kFS_AtoB)
, mCurTime(0.f)
, mColorA(colorA)
, mColorB(colorB)
, mBlendMode(blendMode)
, mTimeA2B(timeA2B)
, mTimeB2A(timeB2A)
, mDoReverse(doReverse)
, mResetTargetWhenDone(resetTargetWhenDone)
, mDepthCompare(depthCompare)
, mDepthUpdate(depthUpdate)
, mDepthBackwards(depthBackwards)
, mReversing(false)
, mEnable(false)
, mDieOnEnd(false)
, mIsFadeOutHelper(false) {}

TUniqueId CScriptColorModulate::FadeInHelper(CStateManager& mgr, const TUniqueId obj,
                                             const float fadetime) {
  const CEntity* ent = mgr.GetObjectById(obj);
  const TAreaId aid = ent ? ent->GetCurrentAreaId() : mgr.GetNextAreaId();
  rstl::string name;
  const TUniqueId ret = mgr.AllocateUniqueId();

  CEntity* colMod = rs_new CScriptColorModulate(
      ret, name, CEntityInfo(aid, NullConnectionList), CColor(1.f, 1.f, 1.f, 0.f),
      CColor(1.f, 1.f, 1.f, 1.f), kBM_Alpha, fadetime, 0.f, false, true, true, true, false, true);

  CScriptColorModulate* mod = static_cast< CScriptColorModulate* >(colMod);
  mod->SetParent(obj);
  mod->SetEnabled(true);
  mod->SetDieOnEnd(true);

  mgr.AddObject(*colMod);

  colMod->Think(0.f, mgr);

  return ret;
}

TUniqueId CScriptColorModulate::FadeOutHelper(CStateManager& mgr, const TUniqueId obj,
                                              const float fadetime) {
  const CEntity* ent = mgr.GetObjectById(obj);
  const TAreaId aid = ent ? ent->GetCurrentAreaId() : mgr.GetNextAreaId();
  rstl::string name;
  const TUniqueId ret = mgr.AllocateUniqueId();

  CEntity* colMod = rs_new CScriptColorModulate(
      ret, name, CEntityInfo(aid, NullConnectionList), CColor(1.f, 1.f, 1.f, 1.f),
      CColor(1.f, 1.f, 1.f, 0.f), kBM_Alpha, fadetime, 0.f, false, false, true, true, false, true);

  CScriptColorModulate* mod = static_cast< CScriptColorModulate* >(colMod);
  mod->SetParent(obj);
  mod->SetEnabled(true);
  mod->SetDieOnEnd(true);
  mod->SetIsFadeOutHelper(true);
  mgr.AddObject(*colMod);

  colMod->Think(0.f, mgr);

  return ret;
}

void CScriptColorModulate::SetTargetFlags(CStateManager& mgr, const CModelFlags& flags) {
  for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
    if (conn->mState != kSS_Play || conn->mMsg != kSM_Activate) {
      continue;
    }

    AUTO(search, mgr.GetIdListForScript(conn->mObjId));

    for (AUTO(it, search.first); it != search.second; ++it) {
      if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(it->second))) {
        act->SetModelFlags(flags);
      }
    }
  }

  if (mParent == kInvalidUniqueId) {
    return;
  }

  if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mParent))) {
    act->SetModelFlags(flags);
  }
}

void CScriptColorModulate::End(CStateManager& mgr) {
  mCurTime = 0.f;
  if (mDoReverse && !mReversing) {
    mReversing = true;
    mFadeState = mFadeState == kFS_AtoB ? kFS_BtoA : kFS_AtoB;
    return;
  }

  mEnable = false;
  mReversing = false;

  if (mResetTargetWhenDone) {
    SetTargetFlags(mgr, CModelFlags(CModelFlags::kT_Opaque, 1.f));
  }

  if (mIsFadeOutHelper) {
    mgr.SendScriptMsgAlways(mParent, GetUniqueId(), kSM_Deactivate);
  }

  SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);

  if (mDieOnEnd) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

CModelFlags CScriptColorModulate::CalculateFlags(const CColor& col) const {
  if (mDepthBackwards) {
    switch (mBlendMode) {
    case kBM_Alpha:
      return CModelFlags::AlphaBlended(col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_Additive:
      return CModelFlags::Additive(col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_Additive2:
      return CModelFlags(CModelFlags::kT_Additive2, col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_Opaque:
      return CModelFlags(CModelFlags::kT_One, col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    case kBM_OpaqueAdd:
      return CModelFlags(CModelFlags::kT_Two, col)
          .DepthCompareUpdate(mDepthCompare, mDepthUpdate)
          .DepthBackwards();
    }
  }
  switch (mBlendMode) {
  case kBM_Alpha:
    if (col == CColor::White()) {
      const bool update = mDepthUpdate;
      const bool compare = mDepthCompare;
      return CModelFlags::Normal().DepthCompareUpdate(compare, update);
    }
    return CModelFlags::AlphaBlended(col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_Additive:
    return CModelFlags::Additive(col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_Additive2:
    return CModelFlags(CModelFlags::kT_Additive2, col)
        .DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_Opaque:
    if (col == CColor::White()) {
      const bool update = mDepthUpdate;
      const bool compare = mDepthCompare;
      return CModelFlags::Normal().DepthCompareUpdate(compare, update);
    }
    return CModelFlags(CModelFlags::kT_One, col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  case kBM_OpaqueAdd:
    return CModelFlags(CModelFlags::kT_Two, col).DepthCompareUpdate(mDepthCompare, mDepthUpdate);
  }
  return CModelFlags::Normal();
}
void CScriptColorModulate::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !mEnable) {
    return;
  }

  mCurTime += dt;

  switch (mFadeState) {
  case kFS_AtoB: {
    CColor lerpedCol =
        CColor::Lerp(mColorA, mColorB,
                     close_enough(mTimeA2B, 0.f) ? 1.f : rstl::min_val(1.f, mCurTime / mTimeA2B));
    SetTargetFlags(mgr, CalculateFlags(lerpedCol));

    if (mCurTime > mTimeA2B) {
      End(mgr);
    }
  } break;
  case kFS_BtoA: {
    CColor lerpedCol =
        CColor::Lerp(mColorB, mColorA,
                     close_enough(mTimeB2A, 0.f) ? 1.f : rstl::min_val(1.f, mCurTime / mTimeB2A));
    SetTargetFlags(mgr, CalculateFlags(lerpedCol));

    if (mCurTime > mTimeB2A) {
      End(mgr);
    }
  } break;
  }
}

void CScriptColorModulate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                           CStateManager& mgr) {
  CEntity::AcceptScriptMsg(msg, uid, mgr);
  if (!GetActive()) {
    return;
  }

  switch (msg) {
  case kSM_Increment: {
    if (mReversing) {
      mFadeState = mFadeState == kFS_AtoB ? kFS_BtoA : kFS_AtoB;
      mReversing = false;
      break;
    }

    if (mEnable) {
      if (mFadeState == kFS_AtoB) {
        mCurTime = 0.f;
      } else {
        mCurTime = mTimeA2B - mTimeA2B * (mCurTime / mTimeB2A);
      }
    } else {
      SetTargetFlags(mgr, CalculateFlags(mColorA));
    }
    mEnable = true;
    mFadeState = kFS_AtoB;
  } break;
  case kSM_Decrement: {
    if (mReversing) {
      mFadeState = mFadeState == kFS_AtoB ? kFS_BtoA : kFS_AtoB;
      mReversing = false;
      break;
    }

    if (mEnable) {
      if (mFadeState == kFS_AtoB) {
        mCurTime = 0.f;
      } else {
        mCurTime = mTimeB2A - mTimeB2A * (mCurTime / mTimeA2B);
      }
    } else {
      SetTargetFlags(mgr, CalculateFlags(mColorB));
    }
    mEnable = true;
    mFadeState = kFS_BtoA;
  } break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptColorModulate)
