#include "MetroidPrime/CGameCubeDoll.hpp"

#include "MetroidPrime/CActorLights.hpp"

#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"

#include "rstl/math.hpp"

#if defined(TARGET_PC)
#include "Metaforce/Display.hpp"
#endif

CGameCubeDoll::CGameCubeDoll()
: x0_model(gpSimplePool->GetObj("CMDL_GameCube"))
, x8_lights(1, CLight::BuildDirectional(CVector3f::Forward(), CColor(0xFFFFFFFF)),
            rstl::rmemory_allocator())
, x18_actorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4,
                                      CActorLights::kDefaultPositionUpdateThreshold, false, false,
                                      false))
, x1c_fader(0.0f)
, x20_24_loaded(false) {
  x0_model.Lock();
}

bool CGameCubeDoll::CheckLoadComplete() {
  if (IsLoaded())
    return true;

  if (!x0_model.IsLoaded()) {
    return false;
  }

  x20_24_loaded = true;
  return true;
}

bool CGameCubeDoll::IsLoaded() const { return x20_24_loaded != false; }

CGameCubeDoll::~CGameCubeDoll() {}

void CGameCubeDoll::UpdateActorLights() {
  (CVector3f::Forward() + CVector3f::Right() * 0.25f + CVector3f::Down() * 0.1f).AsNormalized();

  x8_lights[0] =
      CLight::BuildDirectional(CVector3f::Forward(), CColor(uchar(0xFF), uchar(0xFF), uchar(0xFF)));
  x18_actorLights->BuildFakeLightList(x8_lights, CColor(0.25f, 0.25f, 0.25f, 1.f));
}

void CGameCubeDoll::Touch() {
  if (!CheckLoadComplete())
    return;
  TToken< CModel >(x0_model)->Touch(0);
}

void CGameCubeDoll::Update(float dt) {
  if (!CheckLoadComplete())
    return;
  x1c_fader = rstl::min_val(1.f, (dt * 2.f) + x1c_fader);
  UpdateActorLights();
}

void CGameCubeDoll::Draw(float alpha) {
  if (!IsLoaded())
    return;

  float alphaFader = alpha * x1c_fader;

  gpRender->SetPerspective(55.f, CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight,
                           0.2f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif

  CGraphics::SetViewPointMatrix(CTransform4f::Translate(0.f, -2.f, 0.f));
  x18_actorLights->ActivateLights();

  float f = (CGraphics::GetSecondsMod900() * 360.f) * 0.25f;
  CGraphics::SetModelMatrix(CTransform4f::RotateZ(CRelAngle::FromDegrees(-f)) *
                            CTransform4f::Scale(0.2f));

  TToken< CModel > model(x0_model);
  model->Draw(CModelFlags(CModelFlags::kT_Blend, alphaFader));
  CGraphics::DisableAllLights();
}
