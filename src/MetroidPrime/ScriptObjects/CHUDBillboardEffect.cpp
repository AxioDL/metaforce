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

, mTranslation(translation.GetX(), translation.GetY() + dist, translation.GetZ())
, mLocalScale(CVector3f::ByElementMultiply(scale1, scale0))
, mRenderAsParticleGen(true)
, mEnableRender(false)
, mIsElementGen(false)
, mRunIndefinitely(false)
, mTimeoutTimer(0.f) {

  if (particle) {
    mIsElementGen = true;
    mGenerator = rs_new CElementGen(*particle);
    if (static_cast< CElementGen& >(*mGenerator).IsIndirectTextured())
      ++g_IndirectTexturedBillboardCount;
  } else {
    mGenerator = rs_new CParticleElectric(TToken< CElectricDescription >(*electric));
  }
  ++g_BillboardCount;
  mGenerator->SetModulationColor(color);
  mGenerator->SetLocalScale(mLocalScale);
}

CHUDBillboardEffect::~CHUDBillboardEffect() {
  --g_BillboardCount;
  if (mGenerator->Get4CharId() == 'PART')
    if (static_cast< CElementGen& >(*mGenerator).IsIndirectTextured())
      --g_IndirectTexturedBillboardCount;
}

void CHUDBillboardEffect::AddToRenderer(const CFrustumPlanes& frustum,
                                        const CStateManager& mgr) const {
  if (mEnableRender && mRenderAsParticleGen) {
    gpRender->AddParticleGen(*mGenerator);
  }
}

void CHUDBillboardEffect::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (mgr.GetPlayer()->GetCameraState() == CPlayer::kCS_FirstPerson) {
    CTransform4f camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    mGenerator->SetGlobalTranslation(camXf * mTranslation);
    mGenerator->SetGlobalOrientation(camXf);
    mEnableRender = true;
  } else {
    mEnableRender = false;
  }
  mRenderAsParticleGen = !mgr.RenderLast(GetUniqueId());
}

void CHUDBillboardEffect::Render(const CStateManager& mgr) const {
  if (mEnableRender && !mRenderAsParticleGen) {
    mGenerator->Render();
  }
}

void CHUDBillboardEffect::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    mgr.SetActorAreaId(*this, mgr.GetWorld()->GetCurrentAreaId());
    float oldGenRate = mGenerator->GetGeneratorRate();
    mGenerator->SetGeneratorRate(oldGenRate * CalcGenRate());
    mGenerator->Update(dt);
    mGenerator->SetGeneratorRate(oldGenRate);
    if (!mRunIndefinitely) {
      mTimeoutTimer += dt;
      if (mTimeoutTimer > 30.f) {
        mgr.DeleteObjectRequest(GetUniqueId());
        return;
      }
    }
    if (mGenerator->IsSystemDeletable())
      mgr.DeleteObjectRequest(GetUniqueId());
  }
}

ENTITY_ACCEPT_IMPL(CHUDBillboardEffect)
