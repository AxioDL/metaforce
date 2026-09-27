#include "MetroidPrime/ScriptObjects/CScriptProjectedShadow.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"

#include "Kyoto/Math/CloseEnough.hpp"

CScriptShadowProjector::CScriptShadowProjector(TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const CTransform4f& xf,
                                               const bool active, const CVector3f& offset,
                                               bool persistent, float scale, float f2,
                                               float opacity, float opacityQ, int textureSize)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mScale(scale)
, mOffset(offset)
, mZOffsetAdjust(f2)
, mOpacity(opacity)
, mOpacityRecip(close_enough(opacity, 0.f) ? 1.f : opacityQ / opacity)
, mTarget(kInvalidUniqueId)
, mProjectedShadow(nullptr)
, mTextureSize(textureSize)
, mPersistent(persistent)
, mShadowInvalidated(false) {}

void CScriptShadowProjector::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_InitializedInArea:
    for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
         conn != GetConnectionList().end(); ++conn) {
      if (conn->mState != kSS_Play) {
        continue;
      }

      if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(conn->mObjId)))) {
        if (act->HasModelData()) {
          mTarget = act->GetUniqueId();
          break;
        }
      }
    }
    if (mTarget == kInvalidUniqueId) {
      mgr.DeleteObjectRequest(GetUniqueId());
      break;
    }

  case kSM_Deactivate:
  case kSM_Activate:
    if (GetActive() && mTarget != kInvalidUniqueId && mOpacity > 0.f) {
      mProjectedShadow =
          rs_new CProjectedShadow(mTextureSize, mTextureSize, GetPersistent());
    } else {
      mProjectedShadow = nullptr;
    }

    break;

  case kSM_Decrement:
    if (!GetActive()) {
      return;
    }

    if (mOpacity > 0.f) {
      mShadowInvalidated = true;
    }

    break;

  default:
    break;
  }
}

void CScriptShadowProjector::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptShadowProjector::PreRender(CStateManager& mgr, const CFrustumPlanes&) {
  SetPreRenderClipped(true);
  if (!mProjectedShadow.null()) {
    CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mTarget));
    bool hasModelData;
    if (act != nullptr) {
      hasModelData = false;
      if (act->HasModelData()) {
        hasModelData = true;
      }
    } else {
      mTarget = kInvalidUniqueId;
      return;
    }

    if (hasModelData) {
      if (!act->GetActive()) {
        return;
      }

      if (act->HasAnimation()) {
        act->AnimationData()->PreRender();
      }
      mProjectedShadow->SetOpacity(mOpacity);
      mProjectedShadow->RenderShadowBuffer(mgr, *act->GetModelData(), act->GetTransform(), 0,
                                               mOffset, mScale, mZOffsetAdjust);
    }
  }
}

ENTITY_ACCEPT_IMPL(CScriptShadowProjector)

void CScriptShadowProjector::Think(float dt, CStateManager& mgr) {
  if (GetActive() && mShadowInvalidated) {

    mOpacity = -(mOpacityRecip * dt - mOpacity);
    if (mOpacity <= 0.0f) {
      mOpacity = 0.0f;

      mProjectedShadow = nullptr;

      mShadowInvalidated = false;
      SendScriptMsgs(kSS_Zero, mgr, kSM_None);
    }
  }
}
