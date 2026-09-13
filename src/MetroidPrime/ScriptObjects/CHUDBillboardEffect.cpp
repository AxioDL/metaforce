#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CGameCamera.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "MetaRender/CCubeRenderer.hpp"

#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"

#include "rstl/math.hpp"

int CHUDBillboardEffect::g_BillboardCount = 0;
int CHUDBillboardEffect::g_IndirectTexturedBillboardCount = 0;

float CHUDBillboardEffect::CalcGenRate() {
  float f1 = (g_BillboardCount + g_IndirectTexturedBillboardCount <= 4)
                 ? 0.f
                 : g_BillboardCount * 0.2f + g_IndirectTexturedBillboardCount * 0.1f;
  return 1.f - rstl::min_val(0.8f, f1);
}

const CVector3f& CHUDBillboardEffect::GetScaleForPOV(const CStateManager& mgr) {
  static CVector3f result(0.155f, 1.f, 0.155f);
  return result;
}

float CHUDBillboardEffect::GetNearClipDistance(const CStateManager& mgr) {
  return mgr.GetCameraManager()->GetCurrentCamera(mgr).GetNearClipDistance() + 0.01f;
}

CHUDBillboardEffect::CHUDBillboardEffect(
    const rstl::optional_object< TToken< CGenDescription > >& particle,
    const rstl::optional_object< TToken< CElectricDescription > >& electric, TUniqueId uid,
    bool active, const rstl::string& name, float dist, const CVector3f& scale0, const CColor& color,
    const CVector3f& scale1, const CVector3f& translation)
: CEffect(uid, CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), active, name,
          CTransform4f::Identity())

, xec_translation(translation.GetX(), translation.GetY() + dist, translation.GetZ())
, xf8_localScale(CVector3f::ByElementMultiply(scale1, scale0))
, x104_24_renderAsParticleGen(true)
, x104_25_enableRender(false)
, x104_26_isElementGen(false)
, x104_27_runIndefinitely(false)
, x108_timeoutTimer(0.f) {

  if (particle) {
    x104_26_isElementGen = true;
    xe8_generator = rs_new CElementGen(*particle);
    if (static_cast< CElementGen& >(*xe8_generator).IsIndirectTextured())
      ++g_IndirectTexturedBillboardCount;
  } else {
    xe8_generator = rs_new CParticleElectric(TToken< CElectricDescription >(*electric));
  }
  ++g_BillboardCount;
  xe8_generator->SetModulationColor(color);
  xe8_generator->SetLocalScale(xf8_localScale);
}

CHUDBillboardEffect::~CHUDBillboardEffect() {
  --g_BillboardCount;
  if (xe8_generator->Get4CharId() == 'PART')
    if (static_cast< CElementGen& >(*xe8_generator).IsIndirectTextured())
      --g_IndirectTexturedBillboardCount;
}

void CHUDBillboardEffect::AddToRenderer(const CFrustumPlanes& frustum,
                                        const CStateManager& mgr) const {
  if (x104_25_enableRender && x104_24_renderAsParticleGen) {
    gpRender->AddParticleGen(*xe8_generator);
  }
}

void CHUDBillboardEffect::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
    CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    xe8_generator->SetGlobalTranslation(camXf * xec_translation);
    xe8_generator->SetGlobalOrientation(camXf);
    x104_25_enableRender = true;
  } else {
    x104_25_enableRender = false;
  }
  x104_24_renderAsParticleGen = !mgr.RenderLast(GetUniqueId());
}

void CHUDBillboardEffect::Render(const CStateManager& mgr) const {
  if (x104_25_enableRender && !x104_24_renderAsParticleGen) {
    xe8_generator->Render();
  }
}

void CHUDBillboardEffect::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
    float oldGenRate = xe8_generator->GetGeneratorRate();
    xe8_generator->SetGeneratorRate(oldGenRate * CalcGenRate());
    xe8_generator->Update(dt);
    xe8_generator->SetGeneratorRate(oldGenRate);
    if (!x104_27_runIndefinitely) {
      x108_timeoutTimer += dt;
      if (x108_timeoutTimer > 30.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
    }
    if (xe8_generator->IsSystemDeletable())
      mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(CHUDBillboardEffect)
