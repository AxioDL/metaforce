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

CModelData::~CModelData() {}

CModelData::CModelData() {}
CModelData CModelData::CModelDataNull() {return CModelData();}

CModelData::CModelData(const CStaticRes& res)
: x0_particleScale(res.x4_particleScale)
{
    x1c_normalModel = g_SimplePool->GetObj({SBIG('CMDL'), res.x0_cmdlId});
}

CModelData::CModelData(const CAnimRes& res)
: x0_particleScale(res.x8_particleScale)
{
    TToken<CCharacterFactory> factory = g_CharFactoryBuilder->GetFactory(res);
    xc_animData = factory->CreateCharacter(res.x4_charIdx, res.x14_, factory, res.x1c_);
}

SAdvancementDeltas CModelData::GetAdvancementDeltas(const CCharAnimTime& a,
                                                    const CCharAnimTime& b) const
{
    if (xc_animData)
        return xc_animData->GetAdvancementDeltas(a, b);
    else
        return {};
}

void CModelData::Render(const CStateManager& stateMgr, const zeus::CTransform& xf,
                        const CActorLights* lights, const CModelFlags& drawFlags) const
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

const CSkinnedModel& CModelData::PickAnimatedModel(EWhichModel which) const
{
    const CSkinnedModel* ret = nullptr;
    switch (which)
    {
    case EWhichModel::XRay:
        ret = xc_animData->xf4_xrayModel.get();
    case EWhichModel::Thermal:
        ret = xc_animData->xf8_infraModel.get();
    default: break;
    }
    if (ret)
        return *ret;
    return *xc_animData->xd8_modelData.GetObj();
}

const TLockedToken<CModel>& CModelData::PickStaticModel(EWhichModel which) const
{
    const TLockedToken<CModel>* ret = nullptr;
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
            if (xc_animData && modelSkin.second &&
                g_ResFactory->GetResourceTypeById(modelSkin.second) == SBIG('CSKR'))
            {
                xc_animData->SetXRayModel(g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first}),
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
            if (xc_animData && modelSkin.second &&
                g_ResFactory->GetResourceTypeById(modelSkin.second) == SBIG('CSKR'))
            {
                xc_animData->SetInfraModel(g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first}),
                                           g_SimplePool->GetObj({SBIG('CSKR'), modelSkin.second}));
            }
            else
            {
                x3c_infraModel = g_SimplePool->GetObj({SBIG('CMDL'), modelSkin.first});
            }
        }
    }
}

bool CModelData::IsDefinitelyOpaque(EWhichModel which) const
{
    if (xc_animData)
    {
        const CSkinnedModel& model = PickAnimatedModel(which);
        return model.GetModel()->GetInstance().IsOpaque();
    }
    else
    {
        const TLockedToken<CModel>& model = PickStaticModel(which);
        return model->GetInstance().IsOpaque();
    }
}

bool CModelData::GetIsLoop() const
{
    if (!xc_animData)
        return false;
    return xc_animData->GetIsLoop();
}

float CModelData::GetAnimationDuration(int idx) const
{
    if (!xc_animData)
        return 0.f;
    return xc_animData->GetAnimationDuration(idx);
}

void CModelData::EnableLooping(bool enable)
{
    if (!xc_animData)
        return;
    xc_animData->EnableLooping(enable);
}

void CModelData::AdvanceParticles(const zeus::CTransform& xf, float dt,
                                  CStateManager& stateMgr)
{
    if (!xc_animData)
        return;
    xc_animData->AdvanceParticles(xf, dt, x0_particleScale, stateMgr);
}

zeus::CAABox CModelData::GetBounds() const
{
    if (xc_animData)
    {
        return xc_animData->GetBoundingBox(zeus::CTransform::Scale(x0_particleScale));
    }
    else
    {
        const zeus::CAABox& aabb = x1c_normalModel->GetAABB();
        return zeus::CAABox(aabb.m_min * x0_particleScale, aabb.m_max * x0_particleScale);
    }
}

zeus::CAABox CModelData::GetBounds(const zeus::CTransform& xf) const
{
    zeus::CTransform xf2 = xf * zeus::CTransform::Scale(x0_particleScale);
    if (xc_animData)
        return xc_animData->GetBoundingBox(xf2);
    else
        return x1c_normalModel->GetAABB().getTransformedAABox(xf2);
}

zeus::CTransform CModelData::GetScaledLocatorTransformDynamic(const std::string& name,
                                                              const CCharAnimTime* time) const
{
    zeus::CTransform xf = GetLocatorTransformDynamic(name, time);
    xf.m_origin *= x0_particleScale;
    return xf;
}

zeus::CTransform CModelData::GetScaledLocatorTransform(const std::string& name) const
{
    zeus::CTransform xf = GetLocatorTransform(name);
    xf.m_origin *= x0_particleScale;
    return xf;
}

zeus::CTransform CModelData::GetLocatorTransformDynamic(const std::string& name,
                                                        const CCharAnimTime* time) const
{
    if (xc_animData)
        return xc_animData->GetLocatorTransform(name, time);
    else
        return {};
}

zeus::CTransform CModelData::GetLocatorTransform(const std::string& name) const
{
    if (xc_animData)
        return xc_animData->GetLocatorTransform(name, nullptr);
    else
        return {};
}

SAdvancementDeltas CModelData::AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool flag)
{
    if (xc_animData)
        return xc_animData->AdvanceIgnoreParticles(dt, rand, flag);
    else
        return {};
}

SAdvancementDeltas CModelData::AdvanceAnimation(float dt, CStateManager& stateMgr, bool flag)
{
    if (xc_animData)
        return xc_animData->Advance(dt, x0_particleScale, stateMgr, flag);
    else
        return {};
}

bool CModelData::IsAnimating() const
{
    if (!xc_animData)
        return false;
    return xc_animData->IsAnimating();
}

bool CModelData::IsInFrustum(const zeus::CTransform& xf,
                             const CFrustumPlanes& frustum) const
{
    if (!xc_animData && !x1c_normalModel)
        return true;
    return frustum.BoxInFrustumPlanes(GetBounds(xf));
}

void CModelData::RenderParticles(const CFrustumPlanes& frustum) const
{
    if (xc_animData)
        xc_animData->RenderAuxiliary(frustum);
}

void CModelData::Touch(EWhichModel which, int shaderIdx) const
{
    if (xc_animData)
        xc_animData->Touch(PickAnimatedModel(which), shaderIdx);
    else
        PickStaticModel(which)->Touch(shaderIdx);
}

void CModelData::Touch(const CStateManager& stateMgr, int shaderIdx) const
{
    Touch(GetRenderingModel(stateMgr), shaderIdx);
}

void CModelData::RenderThermal(const zeus::CTransform& xf,
                               const zeus::CColor& a, const zeus::CColor& b) const
{
    CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_particleScale));
    CGraphics::DisableAllLights();
    CModelFlags drawFlags;
    drawFlags.m_extendedShaderIdx = 3;

    if (xc_animData)
    {
        const CSkinnedModel& model = PickAnimatedModel(EWhichModel::Thermal);
        xc_animData->SetupRender(model, {}, nullptr);
        model.Draw(drawFlags);
    }
    else
    {
        const TLockedToken<CModel>& model = PickStaticModel(EWhichModel::Thermal);
        model->Draw(drawFlags);
    }
}

void CModelData::RenderUnsortedParts(EWhichModel which, const zeus::CTransform& xf,
                                     const CActorLights* lights, const CModelFlags& drawFlags) const
{
    if ((x14_25_sortThermal && which == EWhichModel::Thermal) ||
        xc_animData || !x1c_normalModel || drawFlags.m_blendMode > 2)
    {
        ((CModelData*)this)->x14_24_renderSorted = false;
        return;
    }

    CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_particleScale));
    if (lights)
        lights->ActivateLights();
    else
    {
        CGraphics::DisableAllLights();
        // Also set ambient to x18_ambientColor
    }

    PickStaticModel(which)->DrawUnsortedParts(drawFlags);
    // Set ambient to white
    CGraphics::DisableAllLights();
    ((CModelData*)this)->x14_24_renderSorted = true;
}

void CModelData::Render(EWhichModel which, const zeus::CTransform& xf,
                        const CActorLights* lights, const CModelFlags& drawFlags) const
{
    if (x14_25_sortThermal && which == EWhichModel::Thermal)
    {
        zeus::CColor mul(drawFlags.color.a, drawFlags.color.a, drawFlags.color.a, drawFlags.color.a);
        RenderThermal(xf, mul, {0.f, 0.f, 0.f, 0.25f});
    }
    else
    {
        CGraphics::SetModelMatrix(xf * zeus::CTransform::Scale(x0_particleScale));
        if (lights)
            lights->ActivateLights();
        else
        {
            CGraphics::DisableAllLights();
            // Also set ambient to x18_ambientColor
        }

        if (xc_animData)
        {
            xc_animData->Render(PickAnimatedModel(which), drawFlags, {}, nullptr);
        }
        else
        {
            const TLockedToken<CModel>& model = PickStaticModel(which);
            if (x14_24_renderSorted)
                model->DrawSortedParts(drawFlags);
            else
                model->Draw(drawFlags);
        }

        // Set ambient to white
        CGraphics::DisableAllLights();
        ((CModelData*)this)->x14_24_renderSorted = false;
    }
}

}
