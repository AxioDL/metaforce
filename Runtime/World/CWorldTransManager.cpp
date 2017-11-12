#include "CWorldTransManager.hpp"
#include "GuiSys/CGuiTextSupport.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CGameState.hpp"
#include "Character/CAnimPlaybackParms.hpp"
#include "Character/CAssetFactory.hpp"
#include "Character/CCharacterFactory.hpp"
#include "Character/IAnimReader.hpp"
#include "Character/CSkinRules.hpp"
#include "Graphics/CModel.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "Camera/CCameraManager.hpp"
#include "Character/CActorLights.hpp"
#include "GuiSys/CStringTable.hpp"
#include "Audio/CSfxManager.hpp"

namespace urde
{

int CWorldTransManager::GetSuitCharIdx()
{
    CPlayerState& state = *g_GameState->GetPlayerState();
    if (state.IsFusionEnabled())
    {
        switch (state.x20_currentSuit)
        {
        case CPlayerState::EPlayerSuit::Power:
            return 4;
        case CPlayerState::EPlayerSuit::Gravity:
            return 6;
        case CPlayerState::EPlayerSuit::Varia:
            return 7;
        case CPlayerState::EPlayerSuit::Phazon:
            return 8;
        default: break;
        }
    }
    return int(state.x20_currentSuit);
}

CWorldTransManager::SModelDatas::SModelDatas(const CAnimRes& samusRes)
: x0_samusRes(samusRes)
{
    x1a0_lights.reserve(8);
}

void CWorldTransManager::UpdateLights(float dt)
{
    if (!x4_modelData)
        return;

    x4_modelData->x1a0_lights.clear();
    zeus::CVector3f lightPos(0.f, 10.f, 0.f);
    CLight spot = CLight::BuildSpot(lightPos, zeus::CVector3f::skBack, zeus::CColor::skWhite, 90.f);
    spot.SetAttenuation(1.f, 0.f, 0.f);

    CLight s1 = spot;
    s1.SetPosition(lightPos + zeus::CVector3f{0.f, 0.f, 2.f * x18_bgOffset - x1c_bgHeight});

    float z = 1.f;
    float delta = x1c_bgHeight - x18_bgOffset;
    if (!x44_26_goingUp && delta < 2.f)
        z = delta * 0.5f;
    else if (x44_26_goingUp && x18_bgOffset < 2.f)
        z = x18_bgOffset * 0.5f;

    if (z < 1.f)
    {
        CLight s2 = spot;
        float pos = x44_26_goingUp ? x1c_bgHeight : -x1c_bgHeight;
        s2.SetPosition(lightPos + zeus::CVector3f{0.f, 0.f, pos});
        s2.SetColor(zeus::CColor::lerp(zeus::CColor::skBlack, zeus::CColor::skWhite, 1.f - z));
        x4_modelData->x1a0_lights.push_back(std::move(s2));
        s1.SetColor(zeus::CColor::lerp(zeus::CColor::skBlack, zeus::CColor::skWhite, z));
    }

    x4_modelData->x1a0_lights.push_back(std::move(s1));
}

void CWorldTransManager::UpdateDisabled(float)
{
    if (x0_curTime <= 2.f)
        return;
    x44_24_transFinished = true;
}

void CWorldTransManager::UpdateEnabled(float dt)
{
    if (x4_modelData && !x4_modelData->x1c_samusModelData.IsNull())
    {
        if (x44_25_stopSoon && !x4_modelData->x1dc_dissolveStarted && x0_curTime > 2.f)
        {
            x4_modelData->x1dc_dissolveStarted = true;
            x4_modelData->x1d0_dissolveStartTime = x0_curTime;
            x4_modelData->x1d4_dissolveEndTime = 4.f + x0_curTime - 2.f;
            x4_modelData->x1d8_transCompleteTime = 5.f + x0_curTime - 2.f;
        }

        if (x0_curTime > x4_modelData->x1d8_transCompleteTime && x4_modelData->x1dc_dissolveStarted)
            x44_24_transFinished = true;

        x4_modelData->x1c_samusModelData.AdvanceAnimationIgnoreParticles(dt, x20_random, true);
        x4_modelData->x170_gunXf = x4_modelData->x1c_samusModelData.GetScaledLocatorTransform("GUN_LCTR");

        x4_modelData->x1c4_randTimeout -= dt;

        if (x4_modelData->x1c4_randTimeout <= 0.f)
        {
            x4_modelData->x1c4_randTimeout = x20_random.Range(0.016666668f, 0.1f);
            zeus::CVector2f randVec(x20_random.Range(-0.025f, 0.025f), x20_random.Range(-0.075f, 0.075f));
            x4_modelData->x1bc_shakeDelta = (randVec - x4_modelData->x1b4_shakeResult) / x4_modelData->x1c4_randTimeout;
            x4_modelData->x1cc_blurDelta = (x20_random.Range(-2.f, 4.f) - x4_modelData->x1c8_blurResult) / x4_modelData->x1c4_randTimeout;
        }

        x4_modelData->x1b4_shakeResult += x4_modelData->x1bc_shakeDelta * dt;
        x4_modelData->x1c8_blurResult += dt * x4_modelData->x1cc_blurDelta;
    }

    float delta = dt * 50.f;
    if (x44_26_goingUp)
        delta = -delta;

    x18_bgOffset += delta;
    if (x18_bgOffset > x1c_bgHeight)
        x18_bgOffset -= x1c_bgHeight;
    if (x18_bgOffset < 0.f)
        x18_bgOffset += x1c_bgHeight;

    UpdateLights(dt);
}

void CWorldTransManager::UpdateText(float dt)
{
    if (x44_28_textDirty)
    {
        if (xc_strTable.IsLoaded())
        {
            x8_textData->SetText(xc_strTable->GetString(x40_strIdx));
            x3c_sfxInterval = 0.f;
            x44_28_textDirty = false;
        }
        else if (x0_curTime >= x38_textStartTime)
        {
            x38_textStartTime += dt;
        }
    }

    if (x0_curTime >= x38_textStartTime)
    {
        x8_textData->Update(dt);

        float nextSfxInterval = x3c_sfxInterval + g_tweakGui->GetWorldTransManagerCharsPerSfx();
        float printed = x8_textData->GetNumCharsPrinted();
        if (printed >= nextSfxInterval)
        {
            x3c_sfxInterval = nextSfxInterval;
            //CSfxManager::SfxStart(1438, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        }
    }

    if (x44_25_stopSoon)
    {
        if (x8_textData->GetTotalAnimationTime() + 1.f < x8_textData->GetCurTime())
        {
            /* Done typing + 1 sec */
            if (x0_curTime - x34_stopTime > 1.f)
                x44_24_transFinished = true;
        }
        else
        {
            x34_stopTime = x0_curTime;
        }
    }
}

void CWorldTransManager::Update(float dt)
{
    x0_curTime += dt;
    switch (x30_type)
    {
    case ETransType::Disabled:
        UpdateDisabled(dt);
        break;
    case ETransType::Enabled:
        UpdateEnabled(dt);
        break;
    case ETransType::Text:
        UpdateText(dt);
        break;
    }
}

void CWorldTransManager::DrawAllModels()
{
    CActorLights lights(0, zeus::CVector3f::skZero, 4, 4, 0, 0, 0, 0.1f);
    lights.BuildFakeLightList(x4_modelData->x1a0_lights, zeus::CColor{0.1f, 0.1f, 0.1f, 1.0f});

    CModelFlags flags = {};
    flags.m_extendedShader = EExtendedShader::Lighting;

    if (!x4_modelData->x100_bgModelData[0].IsNull())
    {
        zeus::CTransform xf0 = zeus::CTransform::Translate(0.f, 0.f, -(2.f * x1c_bgHeight - x18_bgOffset));
        x4_modelData->x100_bgModelData[0].Render(CModelData::EWhichModel::Normal, xf0, &lights, flags);
    }
    if (!x4_modelData->x100_bgModelData[1].IsNull())
    {
        zeus::CTransform xf1 = zeus::CTransform::Translate(0.f, 0.f, x18_bgOffset - x1c_bgHeight);
        x4_modelData->x100_bgModelData[1].Render(CModelData::EWhichModel::Normal, xf1, &lights, flags);
    }
    if (!x4_modelData->x100_bgModelData[2].IsNull())
    {
        zeus::CTransform xf2 = zeus::CTransform::Translate(0.f, 0.f, x18_bgOffset);
        x4_modelData->x100_bgModelData[2].Render(CModelData::EWhichModel::Normal, xf2, &lights, flags);
    }

    if (!x4_modelData->xb4_platformModelData.IsNull())
    {
        x4_modelData->xb4_platformModelData.Render(CModelData::EWhichModel::Normal, zeus::CTransform::Identity(), &lights, flags);
    }

    if (!x4_modelData->x1c_samusModelData.IsNull())
    {
        x4_modelData->x1c_samusModelData.AnimationData()->PreRender();
        x4_modelData->x1c_samusModelData.Render(CModelData::EWhichModel::Normal, zeus::CTransform::Identity(), &lights, flags);

        if (!x4_modelData->x68_beamModelData.IsNull())
        {
            x4_modelData->x68_beamModelData.Render(CModelData::EWhichModel::Normal, x4_modelData->x170_gunXf, &lights, flags);
        }
    }
}

void CWorldTransManager::DrawFirstPass()
{
    zeus::CTransform translateXf =
        zeus::CTransform::Translate(x4_modelData->x1b4_shakeResult.x,
                                    -3.5f * (1.f - zeus::clamp(0.f, x0_curTime / 10.f, 1.f)) - 3.5f,
                                    x4_modelData->x1b4_shakeResult.y + 2.f);
    zeus::CTransform rotateXf =
        zeus::CTransform::RotateZ(zeus::degToRad(zeus::clamp(0.f, x0_curTime / 25.f, 100.f) *
                                                 360.f + 180.f - 90.f));
    CGraphics::SetViewPointMatrix(rotateXf * translateXf);
    DrawAllModels();
    m_camblur.draw(x4_modelData->x1c8_blurResult);
}

void CWorldTransManager::DrawSecondPass()
{
    const zeus::CVector3f& samusScale = x4_modelData->x0_samusRes.GetScale();
    zeus::CTransform translateXf =
        zeus::CTransform::Translate(-0.1f * samusScale.x,
                                    -0.5f * samusScale.y,
                                    1.5f * samusScale.z);
    zeus::CTransform rotateXf =
        zeus::CTransform::RotateZ(zeus::degToRad(48.f *
            zeus::clamp(0.f, (x0_curTime - x4_modelData->x1d0_dissolveStartTime + 2.f) / 5.f, 1.f) + 180.f - 24.f));
    CGraphics::SetViewPointMatrix(rotateXf * translateXf);
    DrawAllModels();
}

void CWorldTransManager::DrawEnabled()
{
    float wsAspect = CWideScreenFilter::SetViewportToMatch(1.f);

    g_Renderer->SetPerspective(CCameraManager::FirstPersonFOV(),
                               wsAspect,
                               CCameraManager::NearPlane(),
                               CCameraManager::FarPlane());
    g_Renderer->x318_26_requestRGBA6 = true;

    if (x0_curTime <= x4_modelData->x1d0_dissolveStartTime)
        DrawFirstPass();
    else if (x0_curTime >= x4_modelData->x1d4_dissolveEndTime)
        DrawSecondPass();
    else
    {
        float t = zeus::clamp(0.f, (x0_curTime - x4_modelData->x1d0_dissolveStartTime) / 2.f, 1.f);
        DrawFirstPass();
        SClipScreenRect rect(g_Viewport);
        CGraphics::ResolveSpareTexture(rect);
        CGraphics::g_BooMainCommandQueue->clearTarget(true, true);
        DrawSecondPass();
        m_dissolve.drawCropped(zeus::CColor{1.f, 1.f, 1.f, 1.f - t}, 1.f);
    }

    CWideScreenFilter::SetViewportToFull();
    m_widescreen.draw(zeus::CColor::skBlack, 1.f);

    float ftbT = 0.f;
    if (x0_curTime < 0.25f)
        ftbT = 1.f - x0_curTime / 0.25f;
    else if (x0_curTime > x4_modelData->x1d8_transCompleteTime)
        ftbT = 1.f;
    else if (x0_curTime > x4_modelData->x1d8_transCompleteTime - 0.25f)
        ftbT = 1.f - (x4_modelData->x1d8_transCompleteTime - x0_curTime) / 0.25f;
    if (ftbT > 0.f)
        m_fadeToBlack.draw(zeus::CColor{0.f, 0.f, 0.f, ftbT});
}

void CWorldTransManager::DrawDisabled()
{
    m_fadeToBlack.draw(zeus::CColor{0.f, 0.f, 0.f, 0.01f});
}

void CWorldTransManager::DrawText()
{
    float vpAspectRatio = g_Viewport.x8_width / float(g_Viewport.xc_height);
    float width = 448.f * vpAspectRatio;
    CGraphics::SetOrtho(0.f, width, 448.f, 0.f, -4096.f, 4096.f);
    CGraphics::SetViewPointMatrix(zeus::CTransform::Identity());
    CGraphics::SetModelMatrix(zeus::CTransform::Translate((width - 640.f) / 2.f, 0.f, 448.f));
    x8_textData->Render();

    float filterAlpha = 0.f;
    if (x0_curTime < 1.f)
        filterAlpha = 1.f - x0_curTime;
    else if (x44_25_stopSoon)
        filterAlpha = std::min(1.f, x0_curTime - x34_stopTime);

    if (filterAlpha > 0.f)
    {
        zeus::CColor filterColor = x44_27_fadeWhite ? zeus::CColor::skWhite : zeus::CColor::skBlack;
        filterColor.a = filterAlpha;
        m_fadeToBlack.draw(filterColor);
    }
}

void CWorldTransManager::Draw()
{
    if (x30_type == ETransType::Disabled)
        DrawDisabled();
    else if (x30_type == ETransType::Enabled)
        DrawEnabled();
    else if (x30_type == ETransType::Text)
        DrawText();
}

void CWorldTransManager::TouchModels()
{
    if (!x4_modelData)
        return;

    if (x4_modelData->x68_beamModelData.IsNull() &&
        x4_modelData->x14c_beamModel.IsLoaded() &&
        x4_modelData->x14c_beamModel.GetObj())
    {
        x4_modelData->x68_beamModelData = {CStaticRes(x4_modelData->x14c_beamModel.GetObjectTag()->id,
                                                      x4_modelData->x0_samusRes.GetScale()), 2};
    }

    if (x4_modelData->x1c_samusModelData.IsNull() &&
        x4_modelData->x158_suitModel.IsLoaded() &&
        x4_modelData->x158_suitModel.GetObj() &&
        x4_modelData->x164_suitSkin.IsLoaded() &&
        x4_modelData->x164_suitSkin.GetObj())
    {
        CAnimRes animRes(x4_modelData->x0_samusRes.GetId(), GetSuitCharIdx(),
                         x4_modelData->x0_samusRes.GetScale(), x4_modelData->x0_samusRes.GetDefaultAnim(),
                         true);
        x4_modelData->x1c_samusModelData = {animRes, 2};

        CAnimPlaybackParms aData(animRes.GetDefaultAnim(), -1, 1.f, true);
        x4_modelData->x1c_samusModelData.AnimationData()->SetAnimation(aData, false);
    }

    if (!x4_modelData->x1c_samusModelData.IsNull())
        x4_modelData->x1c_samusModelData.Touch(CModelData::EWhichModel::Normal, 0);

    if (!x4_modelData->xb4_platformModelData.IsNull())
        x4_modelData->xb4_platformModelData.Touch(CModelData::EWhichModel::Normal, 0);

    if (!x4_modelData->x100_bgModelData[0].IsNull())
        x4_modelData->x100_bgModelData[0].Touch(CModelData::EWhichModel::Normal, 0);
    if (!x4_modelData->x100_bgModelData[1].IsNull())
        x4_modelData->x100_bgModelData[1].Touch(CModelData::EWhichModel::Normal, 0);
    if (!x4_modelData->x100_bgModelData[2].IsNull())
        x4_modelData->x100_bgModelData[2].Touch(CModelData::EWhichModel::Normal, 0);

    if (!x4_modelData->x68_beamModelData.IsNull())
        x4_modelData->x68_beamModelData.Touch(CModelData::EWhichModel::Normal, 0);
}

void CWorldTransManager::EnableTransition(const CAnimRes& samusRes,
                                          CAssetId platRes, const zeus::CVector3f& platScale,
                                          CAssetId bgRes, const zeus::CVector3f& bgScale, bool goingUp)
{
    x44_25_stopSoon = false;
    x44_26_goingUp = goingUp;
    x30_type = ETransType::Enabled;
    x4_modelData.reset(new SModelDatas(samusRes));

    x8_textData.reset();
    x20_random.SetSeed(99);

    CAssetId beamModelId = g_tweakPlayerRes->GetBeamCineModel(g_GameState->GetPlayerState()->GetCurrentBeam());

    x4_modelData->x14c_beamModel = g_SimplePool->GetObj(SObjectTag{FOURCC('CMDL'), beamModelId});

    TToken<CCharacterFactory> fac = g_CharFactoryBuilder->GetFactory(samusRes);
    const CCharacterInfo& info = fac.GetObj()->GetCharInfo(GetSuitCharIdx());
    x4_modelData->x158_suitModel = g_SimplePool->GetObj(SObjectTag{FOURCC('CMDL'), info.GetModelId()});
    x4_modelData->x164_suitSkin = g_SimplePool->GetObj(SObjectTag{FOURCC('CSKR'), info.GetSkinRulesId()});

    if (platRes != -1)
    {
        x4_modelData->xb4_platformModelData = {CStaticRes(platRes, platScale), 2};
        x4_modelData->xb4_platformModelData.Touch(CModelData::EWhichModel::Normal, 0);
    }

    if (bgRes != -1)
    {
        CStaticRes bg(bgRes, bgScale);
        x4_modelData->x100_bgModelData[0] = bg;
        x4_modelData->x100_bgModelData[0].Touch(CModelData::EWhichModel::Normal, 0);
        x4_modelData->x100_bgModelData[1] = bg;
        x4_modelData->x100_bgModelData[1].Touch(CModelData::EWhichModel::Normal, 0);
        x4_modelData->x100_bgModelData[2] = bg;
        x4_modelData->x100_bgModelData[2].Touch(CModelData::EWhichModel::Normal, 0);
        zeus::CAABox bounds = x4_modelData->x100_bgModelData[0].GetBounds();
        x1c_bgHeight = (bounds.max.z - bounds.min.z) * bgScale.z;
    }
    else
        x1c_bgHeight = 0.f;

    TouchModels();
}

void CWorldTransManager::EnableTransition(CAssetId fontId, CAssetId stringId, u32 strIdx, bool fadeWhite,
                                          float chFadeTime, float chFadeRate, float textStartTime)
{
    x40_strIdx = strIdx;
    x38_textStartTime = textStartTime;
    x44_25_stopSoon = false;
    x30_type = ETransType::Text;

    x4_modelData.reset();
    x44_27_fadeWhite = fadeWhite;

    CGuiTextProperties props(false, true, EJustification::Center, EVerticalJustification::Center);
    x8_textData.reset(new CGuiTextSupport(fontId, props, zeus::CColor::skWhite,
                                          zeus::CColor::skBlack, zeus::CColor::skWhite,
                                          640, 448, g_SimplePool, CGuiWidget::EGuiModelDrawFlags::Additive));

    x8_textData->SetTypeWriteEffectOptions(true, chFadeTime, chFadeRate);
    xc_strTable = g_SimplePool->GetObj(SObjectTag{FOURCC('STRG'), stringId});
    x8_textData->SetText(u"");
}

void CWorldTransManager::StartTextFadeOut()
{
    if (!x44_25_stopSoon)
        x34_stopTime = x0_curTime;
    x44_25_stopSoon = true;
}

void CWorldTransManager::DisableTransition()
{
    x4_modelData.reset();
    x8_textData.reset();
    x44_26_goingUp = false;
}

void CWorldTransManager::StartTransition()
{
    x0_curTime = 0.f;
    x18_bgOffset = 0.f;
    x44_24_transFinished = false;
    x44_28_textDirty = true;
}

void CWorldTransManager::EndTransition()
{
    DisableTransition();
}

bool CWorldTransManager::WaitForModelsAndTextures()
{
    std::vector<SObjectTag> tags = g_SimplePool->GetReferencedTags();
    for (const SObjectTag& tag : tags)
    {
        if (tag.type == FOURCC('TXTR') || tag.type == FOURCC('CMDL'))
            g_SimplePool->GetObj(tag).GetObj();
    }
    return true;
}

void CWorldTransManager::SfxStop()
{
    if (x28_sfxHandle)
    {
        CSfxManager::SfxStop(x28_sfxHandle);
        x28_sfxHandle.reset();
    }
}

void CWorldTransManager::SfxStart()
{
    if (!x28_sfxHandle && x24_sfx != 0xFFFF)
        x28_sfxHandle = CSfxManager::SfxStart(x24_sfx, x2c_volume, x2d_panning, false, 127, true, -1);
}

}
