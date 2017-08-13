#include "CScriptWater.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"

namespace urde
{

const float CScriptWater::kSplashScales[6] =
{
    1.0f, 3.0f, 0.709f,
    1.19f, 0.709f, 1.f
};

CScriptWater::CScriptWater(CStateManager& mgr, TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           const zeus::CVector3f& pos, const zeus::CAABox& box, const urde::CDamageInfo& dInfo,
                           zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool b1, bool b2,
                           CAssetId patternMap1, CAssetId patternMap2, CAssetId colorMap, CAssetId bumpMap, CAssetId envMap,
                           CAssetId envBumpMap, CAssetId unusedMap, const zeus::CVector3f& bumpLightDir, float bumpScale,
                           float f2, float f3, bool active, CFluidPlane::EFluidType fluidType, bool b4, float alpha,
                           const CFluidUVMotion& uvMot, float turbSpeed, float turbDistance, float turbFreqMax,
                           float turbFreqMin, float turbPhaseMax, float turbPhaseMin, float turbAmplitudeMax,
                           float turbAmplitudeMin, const zeus::CColor& c1, const zeus::CColor& c2, CAssetId splashParticle1,
                           CAssetId splashParticle2, CAssetId splashParticle3, CAssetId particle4, CAssetId particle5, s32 i1,
                           s32 visorRunoffSfx, s32 splashSfx1, s32 splashSfx2, s32 splashSfx3, float tileSize,
                           u32 tileSubdivisions, float specularMin, float specularMax, float reflectionSize,
                           float fluidPlaneF2, float reflectionBlend, float slF6, float slF7, float slF8,
                           const zeus::CColor& c3, CAssetId lightmapId, float unitsPerLightmapTexel, float lF2, float lF3,
                           u32, u32, bool, s32, s32, std::unique_ptr<u32[]>&& u32Arr)
: CScriptTrigger(uid, name, info, pos, box, dInfo, orientedForce, triggerFlags, active, false, false),
  x1b8_position(pos), x1c4_extent(box.max - box.min), x1d0_f2(f2), x1d4_position2(pos), x1e0_extent2(box.max - box.min),
  x1ec_damage(dInfo.GetDamage()), x1f0_damage2(dInfo.GetDamage()), x1f4_(f3), x214_(slF6), x218_(slF7), x21c_(slF6),
  x220_(slF7), x224_(slF8), x228_(c3), x22c_splashParticle1Id(splashParticle1), x230_splashParticle2Id(splashParticle2),
  x234_splashParticle3Id(splashParticle3), x238_particle4Id(particle4), x24c_particle5Id(particle5),
  x260_(CSfxManager::TranslateSFXID(i1)), x262_visorRunoffSfx(CSfxManager::TranslateSFXID(visorRunoffSfx)),
  x2a4_c1(c1), x2a8_c2(c2), x2ac_lf2(lF2), x2b0_lf3(lF3), x2b4_((lF2 != 0.f) ? 1.f / lF2 : 0.f),
  x2b8_((lF3 != 0.f) ? 1.f / lF3 : 0.f), x2bc_alpha(alpha), x2c0_tileSize(tileSize)
{
    zeus::CAABox triggerAABB = GetTriggerBoundsWR();
    x2c4_gridDimX = u32((x2c0_tileSize + triggerAABB.max.x - triggerAABB.min.x - 0.01f) / x2c0_tileSize);
    x2c8_gridDimY = u32((x2c0_tileSize + triggerAABB.max.y - triggerAABB.min.y - 0.01f) / x2c0_tileSize);
    x2cc_gridCellCount = (x2c4_gridDimX + 1) * (x2c8_gridDimY + 1);
    x2e8_24_b4 = b4;
    x2e8_27_b2 = b2;
    x2e8_28 = true;

    x1b4_fluidPlane = std::make_unique<CFluidPlaneCPU>(patternMap1, patternMap2, colorMap, bumpMap, envMap, envBumpMap,
                                                       lightmapId, unitsPerLightmapTexel, tileSize, tileSubdivisions,
                                                       fluidType, x2bc_alpha, bumpLightDir, bumpScale, uvMot,
                                                       turbSpeed, turbDistance, turbFreqMax, turbFreqMin, turbPhaseMax,
                                                       turbPhaseMin, turbAmplitudeMax, turbAmplitudeMin, specularMin,
                                                       specularMax, reflectionBlend, reflectionSize, fluidPlaneF2);
    u32Arr.reset();
    x264_splashEffects.resize(3);
    if (x22c_splashParticle1Id.IsValid())
        x264_splashEffects[0].emplace(g_SimplePool->GetObj({FOURCC('PART'), x22c_splashParticle1Id}));
    if (x230_splashParticle2Id.IsValid())
        x264_splashEffects[1].emplace(g_SimplePool->GetObj({FOURCC('PART'), x230_splashParticle2Id}));
    if (x234_splashParticle3Id.IsValid())
        x264_splashEffects[2].emplace(g_SimplePool->GetObj({FOURCC('PART'), x234_splashParticle3Id}));
    if (x238_particle4Id.IsValid())
        x23c_.emplace(g_SimplePool->GetObj({FOURCC('PART'), x238_particle4Id}));
    if (x24c_particle5Id.IsValid())
        x250_visorRunoffEffect.emplace(g_SimplePool->GetObj({FOURCC('PART'), x24c_particle5Id}));
    x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx1));
    x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx2));
    x298_splashSounds.push_back(CSfxManager::TranslateSFXID(splashSfx3));
    SetCalculateLighting(true);
    if (lightmapId.IsValid())
        x90_actorLights->DisableAreaLights();
    x90_actorLights->SetMaxDynamicLights(4);
    x90_actorLights->SetCastShadows(false);
    x90_actorLights->SetAmbientChannelOverflow(false);
    x90_actorLights->SetFindNearestDynamicLights(true);
    x148_24_playerInside = true;
    CalculateRenderBounds();
    xe6_27_ = u8(b1 ? 2 : 1);
    if (!x30_24_active)
    {
        x2bc_alpha = 0.f;
        x214_ = 0.f;
        x218_ = 0.f;
    }
    SetupGrid(true);
}

void CScriptWater::SetupGrid(bool b)
{
    zeus::CAABox triggerAABB = GetTriggerBoundsWR();
    auto dimX = u32((triggerAABB.max.x - triggerAABB.min.x + x2c0_tileSize) / x2c0_tileSize);
    auto dimY = u32((triggerAABB.max.y - triggerAABB.min.y + x2c0_tileSize) / x2c0_tileSize);
    x2e4_gridCellCount2 = x2cc_gridCellCount = (dimX + 1) * (dimY + 1);
    x2dc_.reset();
    if (!x2d8_gridFlags || dimX != x2c4_gridDimX || dimY != x2c8_gridDimY)
        x2d8_gridFlags.reset(new bool[dimX * dimY]);
    x2c4_gridDimX = dimX;
    x2c8_gridDimY = dimY;
    for (int i=0 ; i<x2c8_gridDimY ; ++i)
        for (int j=0 ; j<x2c4_gridDimX ; ++j)
            x2d8_gridFlags[i * x2c4_gridDimX + j] = true;
    if (!x2e0_patchFlags || x2d0_patchDimX != 0 || x2d4_patchDimY != 0)
        x2e0_patchFlags.reset(new u8[32]);
    for (int i=0 ; i<32 ; ++i)
        x2e0_patchFlags[i] = 1;
    x2d4_patchDimY = 0;
    x2d0_patchDimX = 0;
    x2e8_28 = b;
}

void CScriptWater::Think(float dt, CStateManager& mgr) { CScriptTrigger::Think(dt, mgr); }

void CScriptWater::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr)
{
    CScriptTrigger::AcceptScriptMsg(msg, other, mgr);
}

void CScriptWater::PreRender(CStateManager &, const zeus::CFrustum &) {}

void CScriptWater::AddToRenderer(const zeus::CFrustum& /*frustum*/, const CStateManager& mgr) const
{
    zeus::CPlane plane;
    plane.vec = x34_transform.origin.normalized();
    plane.d = x34_transform.origin.z + x130_bounds.max.z;
    zeus::CAABox renderBounds = GetSortingBounds(mgr);
    mgr.AddDrawableActorPlane(*this, plane, renderBounds);
}

void CScriptWater::Render(const CStateManager&) const {}

void CScriptWater::Touch(CActor&, CStateManager&)
{
}

void CScriptWater::CalculateRenderBounds()
{
    zeus::CVector3f aabbMin = x130_bounds.min;
    aabbMin.z = x130_bounds.max.z - 1.f;
    zeus::CVector3f aabbMax = x130_bounds.max;
    aabbMax.z += 1.f;
    zeus::CVector3f transAABBMin = aabbMin + GetTranslation();
    zeus::CVector3f transAABBMax = aabbMax + GetTranslation();
    x9c_renderBounds = zeus::CAABox(transAABBMin, transAABBMax);
}

zeus::CAABox CScriptWater::GetSortingBounds(const CStateManager&) const
{
    return {};
}

EWeaponCollisionResponseTypes CScriptWater::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                     const CWeaponMode&, int) const
{
    return EWeaponCollisionResponseTypes::Water;
}

s16 CScriptWater::GetSplashSound(float dt) const
{
    return x298_splashSounds[GetSplashIndex(dt)];
}

const std::experimental::optional<TLockedToken<CGenDescription>>& CScriptWater::GetSplashEffect(float dt) const
{
    return x264_splashEffects[GetSplashIndex(dt)];
}

float CScriptWater::GetSplashEffectScale(float dt) const
{
    if (std::fabs(dt - 1.f) < 0.00001f)
        return kSplashScales[5];

    u32 idx = GetSplashIndex(dt);
    float s = dt - zeus::floorF(dt * 3.f);
    return ((1.f - s) * (s * kSplashScales[idx * 2])) + kSplashScales[idx];
}

u32 CScriptWater::GetSplashIndex(float dt) const
{
    u32 idx = dt * 3.f;
    return (idx < 3 ? idx : idx - 1);
}

const CScriptWater* CScriptWater::GetNextConnectedWater(const CStateManager& mgr) const
{
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
            continue;
        auto its = mgr.GetIdListForScript(conn.x8_objId);
        if (its.first != mgr.GetIdListEnd())
            if (TCastToConstPtr<CScriptWater> water = mgr.GetObjectById(its.first->second))
                return water.GetPtr();
    }
    return nullptr;
}
}
