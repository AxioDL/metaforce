#include "CModelData.hpp"
#include "CAnimData.hpp"
#include "IAnimReader.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CSkinnedModel.hpp"
#include "Graphics/CFrustumPlanes.hpp"
#include "Graphics/CVertexMorphEffect.hpp"
#include "Editor/ProjectManager.hpp"
#include "CActorLights.hpp"
#include "CStateManager.hpp"
#include "CPlayerState.hpp"
#include "GameGlobalObjects.hpp"
#include "CAssetFactory.hpp"
#include "CCharacterFactory.hpp"
#include "CAdditiveAnimPlayback.hpp"

namespace urde
{
static logvisor::Module Log("urde::CModelData");

CModelData::~CModelData() {}

CModelData::CModelData() {}
CModelData CModelData::CModelDataNull() {return CModelData();}

CModelData::CModelData(const CStaticRes& res)
: x0_particleScale(res.GetScale())
{
    x1c_normalModel = g_SimplePool->GetObj({SBIG('CMDL'), res.GetId()});
    if (!x1c_normalModel)
        Log.report(logvisor::Fatal, "unable to find CMDL %08X", res.GetId());
}

CModelData::CModelData(const CAnimRes& res)
: x0_particleScale(res.GetScale())
{
    TToken<CCharacterFactory> factory = g_CharFactoryBuilder->GetFactory(res);
    x10_animData = factory->CreateCharacter(res.GetCharacterNodeId(), res.CanLoop(), factory, res.GetDefaultAnim());
}

SAdvancementDeltas CModelData::GetAdvancementDeltas(const CCharAnimTime& a,
                                                    const CCharAnimTime& b) const
{
    if (x10_animData)
        return x10_animData->GetAdvancementDeltas(a, b);
    else
        return {};
}

void CModelData::Render(const CStateManager& stateMgr, const zeus::CTransform& xf,
                        const CActorLights* lights, const CModelFlags& drawFlags)
{
    Render(GetRenderingModel(stateMgr), xf, lights, drawFlags);
}

CModelData::EWhichModel CModelData::GetRenderingModel(const CStateManager& stateMgr) const
{
    switch (stateMgr.GetPlayerState()->GetActiveVisor(stateMgr))
    {
    case CPlayerState::EPlayerVisor::XRay:
        return CModelData::EWhichModel::XRay;
    case CPlayerState::EPlayerVisor::Thermal:
        return CModelData::EWhichModel::Thermal;
    default:
        return CModelData::EWhichModel::Normal;
    }
}

CSkinnedModel& CModelData::PickAnimatedModel(EWhichModel which) const
{
    CSkinnedModel* ret = nullptr;
    switch (which)
    {
    case EWhichModel::XRay:
        ret = x10_animData->xf4_xrayModel.get();
    case EWhichModel::Thermal:
        ret = x10_animData->xf8_infraModel.get();
    default: break;
    }
    if (ret)
        return *ret;
    return *x10_animData->xd8_modelData.GetObj();
}

TLockedToken<CModel>& CModelData::PickStaticModel(EWhichModel which)
{
    TLockedToken<CModel>* ret = nullptr;
    switch (which)
    {
    case EWhichModel::XRay:
        ret = &x2c_xrayModel;
    case EWhichModel::Thermal:
        ret = &x3c_infraModel;
    default: break;
    }
    if (ret)
        return *ret;
    return x1c_normalModel;
}

void CModelData::SetXRayModel(const std::pair<ResId, ResId>& modelSkin)
{
    if (modelSkin.first)
    {
        if (g_ResFactory->GetResourceTypeById(modelSkin.first) == SBIG('CMDL'))
        {
            if (x10_animData && modelSkin.second &&
                g_ResFactory->GetResourceTypeById(modelSkin.second) == SBIG('CSKR'))
            {
                x10_animData->SetXRayModel(g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first}),
                                          g_SimplePool->GetObj({SBIG('CSKR'), modelSkin.second}));
            }
            else
            {
                x2c_xrayModel = g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first});
            }
        }
    }
}

void CModelData::SetInfraModel(const std::pair<ResId, ResId>& modelSkin)
{
    if (modelSkin.first)
    {
        if (g_ResFactory->GetResourceTypeById(modelSkin.first) == SBIG('CMDL'))
        {
            if (x10_animData && modelSkin.second &&
                g_ResFactory->GetResourceTypeById(modelSkin.second) == SBIG('CSKR'))
            {
                x10_animData->SetInfraModel(g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first}),
                                           g_SimplePool->GetObj({SBIG('CSKR'), modelSkin.second}));
            }
            else
            {
                x3c_infraModel = g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first});
            }
        }
    }
}

bool CModelData::IsDefinitelyOpaque(EWhichModel which)
{
    if (x10_animData)
    {
        CSkinnedModel& model = PickAnimatedModel(which);
        return model.GetModel()->GetInstance().IsOpaque();
    }
    else
    {
        TLockedToken<CModel>& model = PickStaticModel(which);
        return model->GetInstance().IsOpaque();
    }
}

bool CModelData::GetIsLoop() const
{
    if (!x10_animData)
        return false;
    return x10_animData->GetIsLoop();
}

float CModelData::GetAnimationDuration(int idx) const
{
    if (!x10_animData)
        return 0.f;
    return x10_animData->GetAnimationDuration(idx);
}

void CModelData::EnableLooping(bool enable)
{
    if (!x10_animData)
        return;
    x10_animData->EnableLooping(enable);
}

void CModelData::AdvanceParticles(const zeus::CTransform& xf, float dt,
                                  CStateManager& stateMgr)
{
    if (!x10_animData)
        return;
    x10_animData->AdvanceParticles(xf, dt, x0_particleScale, stateMgr);
}

zeus::CAABox CModelData::GetBounds() const
{
    if (x10_animData)
    {
        return x10_animData->GetBoundingBox(zeus::CTransform::Scale(x0_particleScale));
    }
    else
    {
        const zeus::CAABox& aabb = x1c_normalModel->GetAABB();
        return zeus::CAABox(aabb.min * x0_particleScale, aabb.max * x0_particleScale);
    }
}

zeus::CAABox CModelData::GetBounds(const zeus::CTransform& xf) const
{
    zeus::CTransform xf2 = xf * zeus::CTransform::Scale(x0_particleScale);
    if (x10_animData)
        return x10_animData->GetBoundingBox(xf2);
    else
        return x1c_normalModel->GetAABB().getTransformedAABox(xf2);
}

zeus::CTransform CModelData::GetScaledLocatorTransformDynamic(const std::string& name,
                                                              const CCharAnimTime* time) const
{
    zeus::CTransform xf = GetLocatorTransformDynamic(name, time);
    xf.origin *= x0_particleScale;
    return xf;
}

zeus::CTransform CModelData::GetScaledLocatorTransform(const std::string& name) const
{
    zeus::CTransform xf = GetLocatorTransform(name);
    xf.origin *= x0_particleScale;
    return xf;
}

zeus::CTransform CModelData::GetLocatorTransformDynamic(const std::string& name,
                                                        const CCharAnimTime* time) const
{
    if (x10_animData)
        return x10_animData->GetLocatorTransform(name, time);
    else
        return {};
}

zeus::CTransform CModelData::GetLocatorTransform(const std::string& name) const
{
    if (x10_animData)
        return x10_animData->GetLocatorTransform(name, nullptr);
    else
        return {};
}

SAdvancementDeltas CModelData::AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool flag)
{
    if (x10_animData)
        return x10_animData->AdvanceIgnoreParticles(dt, rand, flag);
    else
        return {};
}

SAdvancementDeltas CModelData::AdvanceAnimation(float dt, CStateManager& stateMgr, TAreaId aid, bool flag)
{
    if (x10_animData)
        return x10_animData->Advance(dt, x0_particleScale, stateMgr, aid, flag);
    else
        return {};
}

bool CModelData::IsAnimating() const
{
    if (!x10_animData)
        return false;
    return x10_animData->IsAnimating();
}

bool CModelData::IsInFrustum(const zeus::CTransform& xf,
                             const CFrustumPlanes& frustum) const
{
    if (!x10_animData && !x1c_normalModel)
        return true;
    return frustum.BoxInFrustumPlanes(GetBounds(xf));
}

void CModelData::RenderParticles(const CFrustumPlanes& frustum) const
{
    if (x10_animData)
        x10_animData->RenderAuxiliary(frustum);
}

void CModelData::Touch(EWhichModel which, int shaderIdx)
{
    if (x10_animData)
        x10_animData->Touch(PickAnimatedModel(which), shaderIdx);
    else
        PickStaticModel(which)->Touch(shaderIdx);
}

void CModelData::Touch(const CStateManager& stateMgr, int shaderIdx)
{
    Touch(GetRenderingModel(stateMgr), shaderIdx);
}

void CModelData::RenderThermal(const zeus::CTransform& xf,
                               const zeus::CColor& a, const zeus::CColor& b)
{
    CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_particleScale));
    CGraphics::DisableAllLights();
    CModelFlags drawFlags;
    drawFlags.m_extendedShaderIdx = 3;

    if (x10_animData)
    {
        CSkinnedModel& model = PickAnimatedModel(EWhichModel::Thermal);
        x10_animData->SetupRender(model, drawFlags, {}, nullptr);
        model.Draw(drawFlags);
    }
    else
    {
        TLockedToken<CModel>& model = PickStaticModel(EWhichModel::Thermal);
        model->Draw(drawFlags);
    }
}

void CModelData::RenderUnsortedParts(EWhichModel which, const zeus::CTransform& xf,
                                     const CActorLights* lights, const CModelFlags& drawFlags)
{
    if ((x14_25_sortThermal && which == EWhichModel::Thermal) ||
        x10_animData || !x1c_normalModel || drawFlags.m_blendMode > 2)
    {
        const_cast<CModelData*>(this)->x14_24_renderSorted = false;
        return;
    }

    CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_particleScale));

    TLockedToken<CModel>& model = PickStaticModel(which);
    if (lights)
        lights->ActivateLights(model->GetInstance());
    else
        model->GetInstance().ActivateLights({});

    model->DrawUnsortedParts(drawFlags);
    // Set ambient to white
    CGraphics::DisableAllLights();
    const_cast<CModelData*>(this)->x14_24_renderSorted = true;
}

void CModelData::Render(EWhichModel which, const zeus::CTransform& xf,
                        const CActorLights* lights, const CModelFlags& drawFlags)
{
    if (x14_25_sortThermal && which == EWhichModel::Thermal)
    {
        zeus::CColor mul(drawFlags.color.a, drawFlags.color.a, drawFlags.color.a, drawFlags.color.a);
        RenderThermal(xf, mul, {0.f, 0.f, 0.f, 0.25f});
    }
    else
    {
        CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_particleScale));

        if (x10_animData)
        {
            CSkinnedModel& model = PickAnimatedModel(which);
            if (lights)
                lights->ActivateLights(model.GetModel()->GetInstance());
            else
                model.GetModel()->GetInstance().ActivateLights({});

            x10_animData->Render(model, drawFlags, {}, nullptr);
        }
        else
        {
            TLockedToken<CModel>& model = PickStaticModel(which);
            if (lights)
                lights->ActivateLights(model->GetInstance());
            else
                model->GetInstance().ActivateLights({});

            if (x14_24_renderSorted)
                model->DrawSortedParts(drawFlags);
            else
                model->Draw(drawFlags);
        }

        // Set ambient to white
        CGraphics::DisableAllLights();
        const_cast<CModelData*>(this)->x14_24_renderSorted = false;
    }
}

}
