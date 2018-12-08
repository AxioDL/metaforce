#include "CArtifactDoll.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde::MP1 {

static const char* ArtifactPieceModels[] = {
    "CMDL_Piece1",  // Truth
    "CMDL_Piece2",  // Strength
    "CMDL_Piece3",  // Elder
    "CMDL_Piece4",  // Wild
    "CMDL_Piece5",  // Lifegiver
    "CMDL_Piece6",  // Warrior
    "CMDL_Piece7",  // Chozo
    "CMDL_Piece8",  // Nature
    "CMDL_Piece9",  // Sun
    "CMDL_Piece10", // World
    "CMDL_Piece11", // Spirit
    "CMDL_Piece12"  // Newborn
};

static const CAssetId ArtifactHeadScans[] = {
    0x32C9DDCE, // Truth
    0xB45DAF60, // Strength
    0x7F017CC5, // Elder
    0x62044C7D, // Wild
    0xA9589FD8, // Lifegiver
    0x2FCCED76, // Warrior
    0xE4903ED3, // Chozo
    0x15C68C06, // Nature
    0xDE9A5FA3, // Sun
    0xFBBE9D9A, // World
    0x30E24E3F, // Spirit
    0xB6763C91  // Newborn
};

static const zeus::CColor ArtifactPreColor = {0.4f, 0.68f, 0.88f, 0.8f};
static const zeus::CColor ArtifactPostColor = {1.f, 0.63f, 0.02f, 1.f};

CArtifactDoll::CArtifactDoll() {
  x10_lights.resize(2, CLight::BuildDirectional(zeus::CVector3f::skForward, zeus::CColor::skWhite));
  x20_actorLights = std::make_unique<CActorLights>(8, zeus::CVector3f::skZero, 4, 4, false, false, false, 0.1f);
  x28_24_loaded = false;
  x0_models.reserve(12);
  for (int i = 0; i < 12; ++i)
    x0_models.push_back(g_SimplePool->GetObj(ArtifactPieceModels[i]));
}

int CArtifactDoll::GetArtifactHeadScanIndex(CAssetId scanId) {
  CAssetId orig = g_ResFactory->TranslateNewToOriginal(scanId);
  for (int i = 0; i < 12; ++i)
    if (ArtifactHeadScans[i] == orig)
      return i;
  return -1;
}

CAssetId CArtifactDoll::GetArtifactHeadScanFromItemType(CPlayerState::EItemType item) {
  if (item < CPlayerState::EItemType::Truth || item > CPlayerState::EItemType::Newborn)
    return -1;
  return g_ResFactory->TranslateOriginalToNew(ArtifactHeadScans[int(item) - 29]);
}

void CArtifactDoll::UpdateArtifactHeadScan(const CStateManager& mgr, float delta) {
  CPlayerState& playerState = *mgr.GetPlayerState();
  for (int i = 0; i < 12; ++i) {
    if (mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType(i + 29))) {
      CAssetId newId = g_ResFactory->TranslateOriginalToNew(ArtifactHeadScans[i]);
      playerState.SetScanTime(newId, std::min(playerState.GetScanTime(newId) + delta, 1.f));
    }
  }
}

void CArtifactDoll::CompleteArtifactHeadScan(const CStateManager& mgr) { UpdateArtifactHeadScan(mgr, 1.f); }

void CArtifactDoll::Draw(float alpha, const CStateManager& mgr, bool inArtifactCategory, int selectedArtifact) {
  if (!IsLoaded())
    return;

  alpha *= x24_fader;
  g_Renderer->SetPerspective(55.f, g_Viewport.x8_width, g_Viewport.xc_height, 0.2f, 4096.f);
  CGraphics::SetViewPointMatrix(zeus::CTransform::Translate(0.f, -10.f, 0.f));

  float angle = CGraphics::GetSecondsMod900() * 2.f * M_PIF * 0.25f;
  CGraphics::SetModelMatrix(zeus::CTransform::RotateX(zeus::degToRad(std::sin(angle) * 8.f)) *
                            zeus::CTransform::RotateZ(zeus::degToRad(std::cos(angle) * 8.f)) *
                            zeus::CTransform::RotateX(M_PIF / 2.f) * zeus::CTransform::Scale(0.2f));

  CPlayerState& playerState = *mgr.GetPlayerState();
  for (int i = 0; i < x0_models.size(); ++i) {
    TLockedToken<CModel>& model = x0_models[i];
    zeus::CColor color = ArtifactPreColor;
    if (playerState.HasPowerUp(CPlayerState::EItemType(i + 29))) {
      if (ArtifactHeadScans[i].IsValid()) {
        CAssetId newId = g_ResFactory->TranslateOriginalToNew(ArtifactHeadScans[i]);
        float interp = (playerState.GetScanTime(newId) - 0.5f) * 2.f;
        if (interp < 0.5f)
          color = zeus::CColor::lerp(ArtifactPreColor, zeus::CColor::skWhite, 2.f * interp);
        else
          color = zeus::CColor::lerp(zeus::CColor::skWhite, ArtifactPostColor, 2.f * (interp - 0.5f));
      } else {
        color = ArtifactPostColor;
      }
    }

    if (inArtifactCategory && i == selectedArtifact) {
      float interp = (std::sin(CGraphics::GetSecondsMod900() * 2.f * M_PIF) + 1.f) * 0.5f;
      color = zeus::CColor::lerp(zeus::CColor::skWhite, color, interp);
      color.a() *= zeus::clamp(0.f, 1.25f - interp, 1.f);
    }

    CModelFlags flags(7, 0, 3, zeus::CColor(1.f, 0.f));
    flags.m_extendedShader = EExtendedShader::SolidColorFrontfaceCullLEqualAlphaOnly;
    x20_actorLights->ActivateLights(model->GetInstance());
    model->Draw(flags);

    flags.x4_color = color;
    flags.x4_color.a() *= alpha;
    flags.m_extendedShader = EExtendedShader::ForcedAdditive;
    model->Draw(flags);
  }
}

void CArtifactDoll::UpdateActorLights() {
  x10_lights[0] = CLight::BuildDirectional(
      (zeus::CVector3f::skForward + 0.25f * zeus::CVector3f::skRight + 0.1f * zeus::CVector3f::skDown).normalized(),
      zeus::CColor::skWhite);
  x10_lights[1] = CLight::BuildDirectional(-zeus::CVector3f::skForward, zeus::CColor::skBlack);
  x20_actorLights->BuildFakeLightList(x10_lights, zeus::CColor(0.25f, 1.f));
}

void CArtifactDoll::Update(float dt, const CStateManager& mgr) {
  if (!CheckLoadComplete())
    return;

  x24_fader = std::min(x24_fader + 2.f * dt, 1.f);
  if (std::fabs(x24_fader - 1.f) < 0.00001f)
    UpdateArtifactHeadScan(mgr, 0.5f * dt * 0.5f);
  UpdateActorLights();
}

void CArtifactDoll::Touch() {
  if (!CheckLoadComplete())
    return;

  for (TLockedToken<CModel>& model : x0_models)
    model->Touch(0);
}

bool CArtifactDoll::CheckLoadComplete() {
  if (IsLoaded())
    return true;

  for (TLockedToken<CModel>& model : x0_models)
    if (!model.IsLoaded())
      return false;

  x28_24_loaded = true;
  return true;
}

} // namespace urde::MP1
