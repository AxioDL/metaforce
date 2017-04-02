#include "CSamusHud.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "World/CPlayer.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTextPane.hpp"
#include "GuiSys/CGuiLight.hpp"
#include "GuiSys/CGuiModel.hpp"
#include "GuiSys/CGuiCamera.hpp"

namespace urde
{
namespace MP1
{

CSamusHud* CSamusHud::g_SamusHud = nullptr;

CSamusHud::CSamusHud(CStateManager& stateMgr)
: x8_targetingMgr(stateMgr),
  x258_frmeHelmet(g_SimplePool->GetObj("FRME_Helmet")),
  x268_frmeBaseHud(g_SimplePool->GetObj("FRME_BaseHud"))
{
    x2e0_26_ = true;
    x2e0_27_energyLow = stateMgr.GetPlayer().IsEnergyLow(stateMgr);
    x33c_lights = std::make_unique<CActorLights>(8, zeus::CVector3f::skZero, 4, 1, true, 0, 0, 0.1f);
    x340_hudLights.resize(3, SCachedHudLight(zeus::CVector3f::skZero, zeus::CColor::skWhite,
                                             0.f, 0.f, 0.f, 0.f));
    x46c_.resize(3);
    x5a4_videoBands.resize(4);
    x5d8_guiLights.resize(4);
    UpdateStateTransition(1.f, stateMgr);

    for (int i=0 ; i<16 ; ++i)
        x5ec_[i] = 5.f * i + 40.f;
    for (int i=0 ; i<64 ; ++i)
        x62c_[i] = -0.5f * i;
    for (int i=0 ; i<32 ; ++i)
        x72c_[i] = 0.5f * i - 8.f;

    x264_loadedFrmeHelmet = x258_frmeHelmet.GetObj();
    x274_loadedFrmeBaseHud = x268_frmeBaseHud.GetObj();
    x2a0_helmetIntf = std::make_unique<CHudHelmetInterface>(*x264_loadedFrmeHelmet);

    rstl::prereserved_vector<bool, 4> hasVisors = BuildPlayerHasVisors(stateMgr);
    x2a4_visorMenu = std::make_unique<CHudVisorBeamMenu>(x274_loadedFrmeBaseHud,
                                                         CHudVisorBeamMenu::EHudVisorBeamMenu::Visor,
                                                         hasVisors);

    rstl::prereserved_vector<bool, 4> hasBeams = BuildPlayerHasBeams(stateMgr);
    x2a8_beamMenu = std::make_unique<CHudVisorBeamMenu>(x274_loadedFrmeBaseHud,
                                                        CHudVisorBeamMenu::EHudVisorBeamMenu::Beam,
                                                        hasBeams);

    x2ac_radarIntf = std::make_unique<CHudRadarInterface>(*x274_loadedFrmeBaseHud, stateMgr);

    InitializeFrameGluePermanent(stateMgr);
    UpdateEnergy(0.f, stateMgr, true);
    UpdateMissile(0.f, stateMgr, true);
    UpdateBallMode(stateMgr, true);
}

rstl::prereserved_vector<bool, 4> CSamusHud::BuildPlayerHasVisors(const CStateManager& mgr)
{
    rstl::prereserved_vector<bool, 4> ret;
    ret[0] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::CombatVisor);
    ret[1] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::XRayVisor);
    ret[2] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ScanVisor);
    ret[3] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::ThermalVisor);
    return ret;
}

rstl::prereserved_vector<bool, 4> CSamusHud::BuildPlayerHasBeams(const CStateManager& mgr)
{
    rstl::prereserved_vector<bool, 4> ret;
    ret[0] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PowerBeam);
    ret[1] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::IceBeam);
    ret[2] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::WaveBeam);
    ret[3] = mgr.GetPlayerState()->HasPowerUp(CPlayerState::EItemType::PlasmaBeam);
    return ret;
}

void CSamusHud::InitializeFrameGluePermanent(const CStateManager& mgr)
{
    x588_base_basewidget_pivot = x274_loadedFrmeBaseHud->FindWidget("basewidget_pivot");
    x58c_helmet_BaseWidget_Pivot = x264_loadedFrmeHelmet->FindWidget("BaseWidget_Pivot");
    x590_base_Model_AutoMapper = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget("Model_AutoMapper"));
    x594_base_textpane_counter = static_cast<CGuiTextPane*>(x274_loadedFrmeBaseHud->FindWidget("textpane_counter"));
    x594_base_textpane_counter->TextSupport()->SetFontColor(g_tweakGuiColors->GetHudCounterFill());
    x594_base_textpane_counter->TextSupport()->SetOutlineColor(g_tweakGuiColors->GetHudCounterOutline());
    x598_base_basewidget_message = x274_loadedFrmeBaseHud->FindWidget("basewidget_message");
    for (CGuiWidget* child = static_cast<CGuiWidget*>(x598_base_basewidget_message->GetChildObject());
         child ; child = static_cast<CGuiWidget*>(child->GetNextSibling()))
        child->SetDepthTest(false);
    x59c_base_textpane_message = static_cast<CGuiTextPane*>(x274_loadedFrmeBaseHud->FindWidget("textpane_message"));
    x5a0_base_model_abutton = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget("model_abutton"));
    for (int i=0 ; i<4 ; ++i)
        x5d8_guiLights[i] = x264_loadedFrmeHelmet->GetFrameLight(i);
    x5d8_guiLights[3]->SetColor(zeus::CColor::skBlack);
    for (int i=0 ; i<4 ; ++i)
    {
        SVideoBand& band = x5a4_videoBands[i];
        band.x0_videoband = static_cast<CGuiModel*>(x274_loadedFrmeBaseHud->FindWidget(hecl::Format("model_videoband%d", i)));
        band.x4_randA = CRandom16::GetRandomNumber()->Range(6.f, 66.f);
        band.x8_randB = CRandom16::GetRandomNumber()->Range(16.f, 256.f);
    }
    x59c_base_textpane_message->SetDepthTest(false);
    x598_base_basewidget_message->SetVisibility(false, ETraversalMode::Children);
    x59c_base_textpane_message->TextSupport()->SetFontColor(g_tweakGuiColors->GetHudMessageFill());
    x59c_base_textpane_message->TextSupport()->SetOutlineColor(g_tweakGuiColors->GetHudMessageOutline());
    x59c_base_textpane_message->TextSupport()->SetScanStates(&g_GameState->GameOptions().GetScanStates());
    x590_base_Model_AutoMapper->SetDepthWrite(true);
    x304_basewidgetIdlePos = x588_base_basewidget_pivot->GetIdlePosition();
    x310_cameraPos = x274_loadedFrmeBaseHud->GetFrameCamera()->GetLocalPosition();
    RefreshHudOptions();
}

void CSamusHud::InitializeFrameGlueMutable(const CStateManager& mgr)
{

}

void CSamusHud::UpdateEnergy(float dt, const CStateManager& mgr, bool init)
{

}

void CSamusHud::UpdateMissile(float dt, const CStateManager& mgr, bool init)
{

}

void CSamusHud::UpdateBallMode(const CStateManager& mgr, bool init)
{

}

EHudState CSamusHud::GetDesiredHudState(const CStateManager& mgr)
{
    if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ||
        mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphing)
        return EHudState::Ball;

    switch (mgr.GetPlayerState()->GetTransitioningVisor())
    {
    case CPlayerState::EPlayerVisor::Combat:
        return EHudState::Combat;
    case CPlayerState::EPlayerVisor::XRay:
        return EHudState::XRay;
    case CPlayerState::EPlayerVisor::Scan:
        return EHudState::Scan;
    case CPlayerState::EPlayerVisor::Thermal:
        return EHudState::Thermal;
    default:
        return EHudState::None;
    }
}

void CSamusHud::UpdateStateTransition(float time, const CStateManager& mgr)
{

}

bool CSamusHud::CheckLoadComplete(CStateManager& stateMgr)
{
    return false;
}

void CSamusHud::OnNewInGameGuiState(EInGameGuiState state, CStateManager& stateMgr)
{

}

void CSamusHud::RefreshHudOptions()
{
    if (x29c_decoIntf)
        x29c_decoIntf->UpdateHudAlpha();
    if (x2a0_helmetIntf)
        x2a0_helmetIntf->UpdateHelmetAlpha();
}

void CSamusHud::Touch()
{

}

zeus::CTransform CSamusHud::BuildFinalCameraTransform(const zeus::CQuaternion& rot,
                                                      const zeus::CVector3f& pos,
                                                      const zeus::CVector3f& camPos)
{
    zeus::CQuaternion invRot = rot.inverse();
    return zeus::CTransform(invRot, invRot.transform(camPos - pos) + pos);
}

void CSamusHud::DisplayHudMemo(const std::u16string& text, const CHUDMemoParms& info)
{

}

void CSamusHud::_DeferHintMemo(ResId strg, u32 timePeriods, const CHUDMemoParms& info)
{

}

}
}
