#include "CPhazonBeam.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "World/CPlayer.hpp"
#include "World/CWorld.hpp"
#include "CProjectileWeapon.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde
{

CPhazonBeam::CPhazonBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId,
                         EMaterialTypes playerMaterial, const zeus::CVector3f& scale)
: CGunWeapon(characterId, type, playerId, playerMaterial, scale),
  x238_aaBoxScale(zeus::CVector3f(-0.14664599f, 0.f, -0.14909725f) * scale.y,
                  zeus::CVector3f(0.14664599f, 0.64619601f, 0.14909725f) * scale.y),
  x250_aaBoxTranslate(zeus::CVector3f(-0.0625f, 0.f, -0.09375f) * scale.y,
                      zeus::CVector3f(0.0625f, -0.25f, 0.09375f) * scale.y)
{
    x21c_phazonVeins = g_SimplePool->GetObj("PhazonVeins");
    x228_phazon2nd1 = g_SimplePool->GetObj("Phazon2nd_1");
    x274_24_loaded = false;
    x274_25_clipWipeActive = true;
    x274_26_veinsAlphaActive = false;
    x274_27_phazonVeinsIdx = false;
    m_aaboxShaderScale.setAABB(x238_aaBoxScale);
    m_aaboxShaderTranslate.setAABB(x250_aaBoxTranslate);
}

void CPhazonBeam::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    TAreaId aid = mgr.GetPlayer().GetAreaIdAlways();
    if (msg == EScriptObjectMessage::Deleted && aid != kInvalidAreaId)
        mgr.WorldNC()->GetArea(aid)->SetWeaponWorldLighting(4.f, 1.f);
}

void CPhazonBeam::StopBeam(CStateManager& mgr, bool b1)
{
    if (x234_chargeFxGen)
        x234_chargeFxGen->SetParticleEmission(false);
}

void CPhazonBeam::UpdateBeam(float dt, const zeus::CTransform& targetXf,
                             const zeus::CVector3f& localBeamPos, CStateManager& mgr)
{
    if (x234_chargeFxGen)
        x234_chargeFxGen->SetParticleEmission(IsFiring());
    CGunWeapon::UpdateMuzzleFx(dt, x4_scale, localBeamPos, IsFiring());
}

void CPhazonBeam::CreateBeam(CStateManager& mgr)
{
    x234_chargeFxGen = std::make_unique<CElementGen>(x228_phazon2nd1, CElementGen::EModelOrientationType::Normal,
                                                     CElementGen::EOptionalSystemFlags::One);
    if (x234_chargeFxGen)
    {
        x234_chargeFxGen->SetGlobalScale(x4_scale);
        x234_chargeFxGen->SetParticleEmission(false);
    }
}

void CPhazonBeam::PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf)
{
    if (IsFiring())
    {
        zeus::CTransform backupView = CGraphics::g_ViewMatrix;
        CGraphics::SetViewPointMatrix(xf.inverse() * backupView);
        CGraphics::SetModelMatrix(zeus::CTransform::Identity());
        CGunWeapon::DrawMuzzleFx(mgr);
        CGraphics::SetViewPointMatrix(backupView);
    }
}

void CPhazonBeam::PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf)
{
    if (x234_chargeFxGen)
        x234_chargeFxGen->Render();
    CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CPhazonBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf)
{
    if (x234_chargeFxGen)
    {
        x234_chargeFxGen->SetGlobalOrientAndTrans(xf);
        x234_chargeFxGen->Update(dt);
    }

    CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

static const u16 kSoundId[] = { 1805, 1767 };

void CPhazonBeam::Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf,
                       CStateManager& mgr, TUniqueId homingTarget, float chargeFactor1, float chargeFactor2)
{
    if (chargeState == EChargeState::Normal)
    {
        ActivateCharge(false, false);
        int count = x278_fireTime > 1.f / 3.f ? 5 : 2;
        int seedOffset = 0;
        for (int i=0 ; i<count ; ++i, seedOffset += 1000)
        {
            CProjectileWeapon::SetGlobalSeed(u16(mgr.GetUpdateFrameIndex() + seedOffset));
            CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, chargeFactor2);
            CProjectileWeapon::SetGlobalSeed(u16(mgr.GetUpdateFrameIndex()));
        }
        x278_fireTime = 0.f;
    }
    else
    {
        CGunWeapon::Fire(underwater, dt, chargeState, xf, mgr, homingTarget, chargeFactor1, chargeFactor2);
    }

    NWeaponTypes::play_sfx(kSoundId[int(chargeState)], underwater, false, 0.165f);
}

void CPhazonBeam::Update(float dt, CStateManager& mgr)
{
    CGunWeapon::Update(dt, mgr);
    x278_fireTime += dt;
    TAreaId aid = mgr.GetPlayer().GetAreaIdAlways();
    if (aid != kInvalidAreaId)
    {
        CGameArea* area = mgr.WorldNC()->GetArea(aid);
        if (x278_fireTime > 1.f / 6.f)
            area->SetWeaponWorldLighting(4.f, 1.f);
        else
            area->SetWeaponWorldLighting(4.f, 0.9f);
    }

    if (!IsLoaded())
    {
        if (CGunWeapon::IsLoaded() && !x274_24_loaded)
        {
            x274_24_loaded = x228_phazon2nd1.IsLoaded() && x21c_phazonVeins.IsLoaded();
            if (x274_24_loaded)
            {
                CreateBeam(mgr);
                x224_phazonVeinsData = std::make_unique<CModelData>(
                    CStaticRes(NWeaponTypes::get_asset_id_from_name(x274_27_phazonVeinsIdx ?
                                                                    "PhazonVeins_2" : "PhazonVeins"), x4_scale));
                x21c_phazonVeins.Unlock();
                x274_25_clipWipeActive = true;
            }
        }
    }

    if (x274_25_clipWipeActive)
    {
        x268_clipWipeScale += 0.75f * dt;
        if (x268_clipWipeScale > 1.f)
            x268_clipWipeScale = 1.f;
        if (x268_clipWipeScale > 0.4f)
        {
            if (x26c_clipWipeTranslate < 0.5f)
                x26c_clipWipeTranslate += 0.75f * dt;
            else
                x274_25_clipWipeActive = false;
        }
    }
    else if (x274_26_veinsAlphaActive)
    {
        x270_indirectAlpha = x10_solidModelData->GetLocatorTransform("phazonScale_LCTR_SDK").origin.y;
    }
}

void CPhazonBeam::Load(CStateManager& mgr, bool subtypeBasePose)
{
    CGunWeapon::Load(mgr, subtypeBasePose);
    x228_phazon2nd1.Lock();
    x274_27_phazonVeinsIdx = (mgr.GetActiveRandom()->Next() & 0x2) != 0;
    x21c_phazonVeins = g_SimplePool->GetObj(x274_27_phazonVeinsIdx ? "PhazonVeins_2" : "PhazonVeins");
    x21c_phazonVeins.Lock();
}

void CPhazonBeam::ReInitVariables()
{
    x268_clipWipeScale = 0.f;
    x26c_clipWipeTranslate = 0.f;
    x270_indirectAlpha = 1.f;
    x234_chargeFxGen.reset();
    x224_phazonVeinsData.reset();
    x274_24_loaded = false;
    x274_25_clipWipeActive = true;
    x274_26_veinsAlphaActive = false;
    x1cc_enabledSecondaryEffect = ESecondaryFxType::None;
}

void CPhazonBeam::Unload(CStateManager& mgr)
{
    CGunWeapon::Unload(mgr);
    x228_phazon2nd1.Unlock();
    x21c_phazonVeins.Unlock();
    ReInitVariables();
}

bool CPhazonBeam::IsLoaded() const
{
    return CGunWeapon::IsLoaded() && x274_24_loaded;
}

void CPhazonBeam::DrawClipScaleCube() const
{
    // Render AABB as completely transparent object, only modifying Z-buffer
    m_aaboxShaderScale.draw(zeus::CColor::skClear);
}

void CPhazonBeam::DrawClipTranslateCube() const
{
    // Render AABB as completely transparent object, only modifying Z-buffer
    m_aaboxShaderTranslate.draw(zeus::CColor::skClear);
}

void CPhazonBeam::Draw(bool drawSuitArm, const CStateManager& mgr, const zeus::CTransform& xf,
                       const CModelFlags& flags, const CActorLights* lights) const
{
    CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
    bool drawIndirect = visor == CPlayerState::EPlayerVisor::Combat || visor == CPlayerState::EPlayerVisor::Scan;

    if (drawIndirect)
    {
        CGraphics::ResolveSpareTexture(g_Viewport);
        CModelFlags tmpFlags = flags;
        tmpFlags.m_extendedShader = EExtendedShader::SolidColorBackfaceCullLEqualAlphaOnly;
        CGunWeapon::Draw(drawSuitArm, mgr, xf, tmpFlags, lights);
    }

    CGunWeapon::Draw(drawSuitArm, mgr, xf, flags, lights);

    if (drawIndirect)
    {
        g_Renderer->DrawPhazonSuitIndirectEffect(zeus::CColor(0.3f * x270_indirectAlpha, 0.6f * x270_indirectAlpha,
                                                              x270_indirectAlpha, 0.5f * x270_indirectAlpha),
                                                 {}, zeus::CColor::skWhite, 1.f, 0.f, 0.f, 0.f);
    }

    if (x224_phazonVeinsData)
    {
        zeus::CTransform modelXf = xf * x10_solidModelData->GetScaledLocatorTransform("elbow");
        if (x274_25_clipWipeActive)
        {
            CGraphics::SetModelMatrix(modelXf * zeus::CTransform::Scale(1.f - x268_clipWipeScale));
            DrawClipScaleCube();
            CGraphics::SetModelMatrix(modelXf * zeus::CTransform::Translate(0.f, x26c_clipWipeTranslate, 0.f));
            DrawClipTranslateCube();
        }
        if (x274_26_veinsAlphaActive)
        {
            CModelFlags useFlags(5, 0, 3, zeus::CColor(1.f, 0.5f * x270_indirectAlpha));
            x224_phazonVeinsData->Render(mgr, xf, lights, useFlags);
        }
        else
        {
            x224_phazonVeinsData->Render(mgr, xf, lights, flags);
        }
    }
}

void CPhazonBeam::DrawMuzzleFx(const CStateManager& mgr) const
{
    if (IsFiring())
        CGunWeapon::DrawMuzzleFx(mgr);
}

}