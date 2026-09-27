#include "MetroidPrime/ScriptObjects/CScriptSpiderBallAttractionSurface.hpp"

#include "MetroidPrime/CActorParameters.hpp"

CScriptSpiderBallAttractionSurface::CScriptSpiderBallAttractionSurface(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CVector3f& scale, const bool active)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId)
, mScale(scale)
, mAabb(CAABox(CVector3f(-(scale.GetX() * 0.5f), -(scale.GetY() * 0.5f), -(scale.GetZ() * 0.5f)),
                  CVector3f(scale.GetX() * 0.5f, scale.GetY() * 0.5f, scale.GetZ() * 0.5f))
               .GetTransformedAABox(xf.GetRotation())) {}

CScriptSpiderBallAttractionSurface::~CScriptSpiderBallAttractionSurface() {}

void CScriptSpiderBallAttractionSurface::Touch(CActor& actor, CStateManager& mgr) {
  // Empty
}

rstl::optional_object< CAABox > CScriptSpiderBallAttractionSurface::GetTouchBounds() const {
  if (GetActive()) {
    return CAABox(mAabb.GetMinPoint() + GetTranslation(),
                  mAabb.GetMaxPoint() + GetTranslation());
  }
  return rstl::optional_object_null();
}

void CScriptSpiderBallAttractionSurface::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                                         CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
}

void CScriptSpiderBallAttractionSurface::Think(float dt, CStateManager& mgr) {
  // Empty
}

ENTITY_ACCEPT_IMPL(CScriptSpiderBallAttractionSurface)
