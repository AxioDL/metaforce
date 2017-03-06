#include "CMorphBallShadow.hpp"
#include "CStateManager.hpp"
#include "World/CWorld.hpp"
#include "GameGlobalObjects.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "World/CPlayer.hpp"
#include "Particle/CGenDescription.hpp"

namespace urde
{

void CMorphBallShadow::GatherAreas(const CStateManager& mgr)
{
    x18_areas.clear();
    for (const CGameArea* area = mgr.GetWorld()->GetChainHead(EChain::Alive);
         area != CWorld::GetAliveAreasEnd();
         area = area->GetNext())
    {
        CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::NotOccluded;
        if (area->IsPostConstructed())
            occState = area->GetPostConstructed()->x10dc_occlusionState;
        if (occState == CGameArea::EOcclusionState::Occluded)
            x18_areas.push_back(area->GetAreaId());
    }
}

void CMorphBallShadow::RenderIdBuffer(const zeus::CAABox& aabb, const CStateManager& mgr, CPlayer& player)
{
    xb8_shadowVolume = aabb;
    x0_actors.clear();
    x18_areas.clear();
    x30_worldModelBits.clear();
    g_Renderer->x318_26_requestRGBA6 = true;

    if (!g_Renderer->x318_27_currentRGBA6)
    {
        xd0_hasIds = false;
        return;
    }

    GatherAreas(mgr);

    g_Renderer->BindBallShadowIdTarget();
    CGraphics::g_BooMainCommandQueue->clearTarget();

    zeus::CTransform backupViewMtx = CGraphics::g_ViewMatrix;
    CGraphics::CProjectionState backupProjection = CGraphics::g_Proj;
    zeus::CVector2f backupDepth = CGraphics::g_CachedDepthRange;
    zeus::CTransform viewMtx(zeus::CVector3f::skRight, zeus::CVector3f::skDown, zeus::CVector3f::skForward,
                             zeus::CVector3f((aabb.min.x + aabb.max.x) * 0.5f,
                                             (aabb.min.y + aabb.max.y) * 0.5f,
                                             aabb.max.z));

    CGraphics::SetDepthRange(0.f, 1.f);
    float vpX = (aabb.max.x - aabb.min.x) * 0.5f;
    float vpY = (aabb.max.y - aabb.min.y) * 0.5f;
    float vpZ = (aabb.max.z - aabb.min.z) + FLT_EPSILON;
    CGraphics::SetOrtho(-vpX, vpX, vpY, -vpY, 0.f, vpZ);

    rstl::reserved_vector<TUniqueId, 1024> nearItems;
    mgr.BuildNearList(nearItems, aabb, CMaterialFilter::skPassEverything, &player);

    CGraphics::SetViewPointMatrix(viewMtx);

    int alphaVal = 4;
    for (TUniqueId id : nearItems)
    {
        if (alphaVal > 255)
            break;

        const CActor* actor = static_cast<const CActor*>(mgr.GetObjectById(id));
        if (!actor || !actor->CanDrawStatic())
            continue;

        x0_actors.push_back(actor);

        const CModelData* modelData = actor->GetModelData();
        zeus::CTransform modelXf = actor->GetTransform() * zeus::CTransform::Scale(modelData->GetScale());
        CGraphics::SetModelMatrix(modelXf);

        CModelFlags flags(0, 0, 3, zeus::CColor{1.f, 1.f, 1.f, alphaVal / 255.f});
        flags.m_extendedShaderIdx = 5; // Do solid color draw
        const CBooModel& model = *modelData->PickStaticModel(CModelData::EWhichModel::Normal);
        const_cast<CBooModel&>(model).VerifyCurrentShader(flags.m_matSetIdx);
        model.DrawNormal(flags, nullptr, nullptr);
        alphaVal += 4;
    }

    CGraphics::SetModelMatrix(zeus::CTransform::Identity());

    g_Renderer->FindOverlappingWorldModels(x30_worldModelBits, aabb);
    alphaVal = g_Renderer->DrawOverlappingWorldModelIDs(alphaVal, x30_worldModelBits, aabb);

    g_Renderer->ResolveBallShadowIdTarget();

    g_Renderer->BindMainDrawTarget();
    CGraphics::SetViewPointMatrix(backupViewMtx);
    CGraphics::SetProjectionState(backupProjection);
    CGraphics::SetDepthRange(backupDepth[0], backupDepth[1]);

    xd0_hasIds = alphaVal != 4;
}

bool CMorphBallShadow::AreasValid(const CStateManager& mgr) const
{
    auto it = x18_areas.begin();
    for (const CGameArea* area = mgr.GetWorld()->GetChainHead(EChain::Alive);
         area != CWorld::GetAliveAreasEnd();
         area = area->GetNext())
    {
        CGameArea::EOcclusionState occState = CGameArea::EOcclusionState::NotOccluded;
        if (area->IsPostConstructed())
            occState = area->GetPostConstructed()->x10dc_occlusionState;
        if (occState != CGameArea::EOcclusionState::Occluded)
            continue;
        if (it == x18_areas.end())
            return false;
        if (*it != area->GetAreaId())
            return false;
        ++it;
    }
    return true;
}

void CMorphBallShadow::Render(const CStateManager& mgr, float alpha)
{
    if (!xd0_hasIds || !AreasValid(mgr))
        return;

    CModelFlags flags;
    flags.color.a = alpha;
    flags.m_extendedShaderIdx = 6;

    int alphaVal = 4;
    for (const CActor* actor : x0_actors)
    {
        const CModelData* modelData = actor->GetModelData();
        zeus::CTransform modelXf = actor->GetTransform() * zeus::CTransform::Scale(modelData->GetScale());
        CGraphics::SetModelMatrix(modelXf);

        flags.color.r = alphaVal / 255.f;
        const CBooModel& model = *modelData->PickStaticModel(CModelData::EWhichModel::Normal);
        const_cast<CBooModel&>(model).VerifyCurrentShader(flags.m_matSetIdx);
        model.DrawNormal(flags, nullptr, nullptr);
        alphaVal += 4;
    }

    CGraphics::SetModelMatrix(zeus::CTransform::Identity());
    g_Renderer->DrawOverlappingWorldModelShadows(alphaVal, x30_worldModelBits, xb8_shadowVolume, alpha);
}

}
