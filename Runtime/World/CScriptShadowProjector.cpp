#include "Runtime/World/CScriptShadowProjector.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CProjectedShadow.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

CScriptShadowProjector::CScriptShadowProjector(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, bool active, const zeus::CVector3f& offset,
                                               bool persistent, float scale, float f2, float opacity, float opacityQ,
                                               s32 textureSize)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_scale(scale)
, xec_offset(offset)
, xf8_zOffsetAdjust(f2)
, xfc_opacity(opacity)
, x100_opacityRecip(opacity < 0.00001 ? 1.f : opacityQ / opacity)
, x10c_textureSize(textureSize)
, x110_24_persistent(persistent) {}

void CScriptShadowProjector::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptShadowProjector::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !x110_25_shadowInvalidated) {
    return;
  }

  xfc_opacity = -(x100_opacityRecip * dt - xfc_opacity);
  if (xfc_opacity > 0.f) {
    return;
  }
  xfc_opacity = 0.f;
  x108_projectedShadow.reset();

  x110_25_shadowInvalidated = false;
  SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
}

void CScriptShadowProjector::CreateProjectedShadow() {
  if (!GetActive() || x104_target == kInvalidUniqueId || xfc_opacity <= 0.f) {
    x108_projectedShadow.reset();
  } else {
    x108_projectedShadow.reset(new CProjectedShadow(x10c_textureSize, x10c_textureSize, x110_24_persistent));
  }
}

void CScriptShadowProjector::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::InitializedInArea:
    for (const SConnection& conn : x20_conns) {
      if (conn.x0_state != EScriptObjectState::Play) {
        continue;
      }
      const CActor* act = TCastToConstPtr<CActor>(mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)));
      if (act) {
        const CModelData* mData = act->GetModelData();
        if (mData && (mData->GetAnimationData() || mData->HasNormalModel())) {
          x104_target = act->GetUniqueId();
          break;
        }
      }
    }
    if (x104_target == kInvalidUniqueId) {
      mgr.FreeScriptObject(GetUniqueId());
      break;
    }
    [[fallthrough]];
  case EScriptObjectMessage::Deactivate:
  case EScriptObjectMessage::Activate:
    CreateProjectedShadow();
    break;

  case EScriptObjectMessage::Decrement:
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

void CScriptShadowProjector::PreRender(CStateManager&, const zeus::CFrustum&) {}
} // namespace metaforce
