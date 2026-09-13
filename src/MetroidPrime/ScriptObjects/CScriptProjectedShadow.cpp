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
, xe8_scale(scale)
, xec_offset(offset)
, xf8_zOffsetAdjust(f2)
, xfc_opacity(opacity)
, x100_opacityRecip(close_enough(opacity, 0.f) ? 1.f : opacityQ / opacity)
, x104_target(kInvalidUniqueId)
, x108_projectedShadow(nullptr)
, x10c_textureSize(textureSize)
, x110_24_persistent(persistent)
, x110_25_shadowInvalidated(false) {}

void CScriptShadowProjector::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_InitializedInArea:
    for (rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
         conn != GetConnectionList().end(); ++conn) {
      if (conn->x0_state != kSS_Play) {
        continue;
      }

      if (CActor* act = TCastToPtr< CActor >(mgr.ObjectById(mgr.GetIdForScript(conn->x8_objId)))) {
        if (act->HasModelData()) {
          x104_target = act->GetUniqueId();
          break;
        }
      }
    }
    if (x104_target == kInvalidUniqueId) {
      mgr.DeleteObjectRequest(GetUniqueId());
      break;
    }

  case kSM_Deactivate:
  case kSM_Activate:
    if (GetActive() && x104_target != kInvalidUniqueId && xfc_opacity > 0.f) {
      x108_projectedShadow =
          rs_new CProjectedShadow(x10c_textureSize, x10c_textureSize, GetPersistent());
    } else {
      x108_projectedShadow = nullptr;
    }

    break;

  case kSM_Decrement:
    if (!GetActive()) {
      return;
    }

    if (xfc_opacity > 0.f) {
      x110_25_shadowInvalidated = true;
    }

    break;

  default:
    break;
  }
}

void CScriptShadowProjector::AddToRenderer(const CFrustumPlanes&, const CStateManager&) const {}

void CScriptShadowProjector::PreRender(CStateManager& mgr, const CFrustumPlanes&) {
  SetPreRenderClipped(true);
  if (!x108_projectedShadow.null()) {
    CActor* act = TCastToPtr< CActor >(mgr.ObjectById(x104_target));
    bool hasModelData;
    if (act != nullptr) {
      hasModelData = false;
      if (act->HasModelData()) {
        hasModelData = true;
      }
    } else {
      x104_target = kInvalidUniqueId;
      return;
    }

    if (hasModelData) {
      if (!act->GetActive()) {
        return;
      }

      if (act->HasAnimation()) {
        act->AnimationData()->PreRender();
      }
      x108_projectedShadow->SetOpacity(xfc_opacity);
      x108_projectedShadow->RenderShadowBuffer(mgr, *act->GetModelData(), act->GetTransform(), 0,
                                               xec_offset, xe8_scale, xf8_zOffsetAdjust);
    }
  }
}

ENTITY_ACCEPT_IMPL(CScriptShadowProjector)

void CScriptShadowProjector::Think(float dt, CStateManager& mgr) {
  if (GetActive() && x110_25_shadowInvalidated) {

    xfc_opacity = -(x100_opacityRecip * dt - xfc_opacity);
    if (xfc_opacity <= 0.0f) {
      xfc_opacity = 0.0f;

      x108_projectedShadow = nullptr;

      x110_25_shadowInvalidated = false;
      SendScriptMsgs(kSS_Zero, mgr, kSM_None);
    }
  }
}
