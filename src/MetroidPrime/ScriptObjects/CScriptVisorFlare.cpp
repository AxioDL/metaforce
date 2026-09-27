#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

// TODO inline depth hack
CScriptVisorFlare::CScriptVisorFlare(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const bool active,
                                     const CVector3f& pos, CVisorFlare::EBlendMode blendMode,
                                     bool b1, float f1, float f2, float f3, uint w1, uint w2,
                                     const rstl::vector< CVisorFlare::CFlareDef >& flares)
: CActor(uid, active, name, info, CTransform4f::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(kMT_NoStepLogic), CActorParameters::None(), kInvalidUniqueId)
, mFlare(blendMode, b1, f1, f2, f3, w1, w2, flares)
, mNotInRenderLast(true) {
  SetThermalFlags(kTF_Hot);
}

CScriptVisorFlare::~CScriptVisorFlare() {}

ENTITY_ACCEPT_IMPL(CScriptVisorFlare)

void CScriptVisorFlare::Think(float dt, CStateManager& stateMgr) {
  if (GetActive()) {
    mFlare.Update(dt, GetTranslation(), this, stateMgr);
  }
}

void CScriptVisorFlare::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId,
                                        CStateManager& stateMgr) {
  CActor::AcceptScriptMsg(msg, objId, stateMgr);
}

void CScriptVisorFlare::PreRender(CStateManager& stateMgr, const CFrustumPlanes&) {
  mNotInRenderLast = !stateMgr.RenderLast(GetUniqueId());
}

void CScriptVisorFlare::AddToRenderer(const CFrustumPlanes&, const CStateManager& stateMgr) const {
  if (mNotInRenderLast) {
    EnsureRendered(stateMgr, stateMgr.GetPlayer()->GetTranslation(), GetSortingBounds(stateMgr));
  }
}

void CScriptVisorFlare::Render(const CStateManager& stateMgr) const {
  mFlare.Render(GetTranslation(), stateMgr);
}
