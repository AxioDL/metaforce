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
: mModel(gpSimplePool->GetObj("CMDL_GameCube"))
, mLights(1, CLight::BuildDirectional(CVector3f::Forward(), CColor(0xFFFFFFFF)),
            rstl::rmemory_allocator())
, mActorLights(rs_new CActorLights(8, CVector3f::Zero(), 4, 4,
                                      CActorLights::kDefaultPositionUpdateThreshold, false, false,
                                      false))
, mFader(0.0f)
, mLoaded(false) {
  mModel.Lock();
}

bool CGameCubeDoll::CheckLoadComplete() {
  if (IsLoaded())
    return true;

  if (!mModel.IsLoaded()) {
    return false;
  }

  mLoaded = true;
  return true;
}

bool CGameCubeDoll::IsLoaded() const { return mLoaded != false; }

CGameCubeDoll::~CGameCubeDoll() {}

void CGameCubeDoll::UpdateActorLights() {
  (CVector3f::Forward() + CVector3f::Right() * 0.25f + CVector3f::Down() * 0.1f).AsNormalized();

  mLights[0] =
      CLight::BuildDirectional(CVector3f::Forward(), CColor(uchar(0xFF), uchar(0xFF), uchar(0xFF)));
  mActorLights->BuildFakeLightList(mLights, CColor(0.25f, 0.25f, 0.25f, 1.f));
}

void CGameCubeDoll::Touch() {
  if (!CheckLoadComplete())
    return;
  TToken< CModel >(mModel)->Touch(0);
}

void CGameCubeDoll::Update(float dt) {
  if (!CheckLoadComplete())
    return;
  mFader = rstl::min_val(1.f, (dt * 2.f) + mFader);
  UpdateActorLights();
}

void CGameCubeDoll::Draw(float alpha) {
  if (!IsLoaded())
    return;

  float alphaFader = alpha * mFader;

  gpRender->SetPerspective(55.f, CGraphics::GetViewport().mWidth, CGraphics::GetViewport().mHeight,
                           0.2f, 4096.f);
#if defined(TARGET_PC)
  metaforce::AdjustUiProjection();
#endif

  CGraphics::SetViewPointMatrix(CTransform4f::Translate(0.f, -2.f, 0.f));
  mActorLights->ActivateLights();

  float f = (CGraphics::GetSecondsMod900() * 360.f) * 0.25f;
  CGraphics::SetModelMatrix(CTransform4f::RotateZ(CRelAngle::FromDegrees(-f)) *
                            CTransform4f::Scale(0.2f));

  TToken< CModel > model(mModel);
  model->Draw(CModelFlags(CModelFlags::kT_Blend, alphaFader));
  CGraphics::DisableAllLights();
}
