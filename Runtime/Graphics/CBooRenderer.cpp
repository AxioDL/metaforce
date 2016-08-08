#if _WIN32
#include <D3Dcommon.h>
#endif
#include "GameGlobalObjects.hpp"
#include "CBooRenderer.hpp"
#include "CTexture.hpp"
#include "CModel.hpp"
#include "Particle/CParticleGen.hpp"
#include "CMetroidModelInstance.hpp"
#include "Collision/CAreaOctTree.hpp"

#define FOGVOL_RAMP_RES 256
#define SPHERE_RAMP_RES 32

namespace urde
{

static rstl::reserved_vector<CDrawable, 50> sDataHolder;
static rstl::reserved_vector<rstl::reserved_vector<CDrawable*, 128>, 50> sBucketsHolder;
static rstl::reserved_vector<CDrawablePlaneObject, 8> sPlaneObjectDataHolder;
static rstl::reserved_vector<u16, 8> sPlaneObjectBucketHolder;

rstl::reserved_vector<u16, 50> Buckets::sBucketIndex;
rstl::reserved_vector<CDrawable, 50>* Buckets::sData = nullptr;
rstl::reserved_vector<rstl::reserved_vector<CDrawable*, 128>, 50>* Buckets::sBuckets = nullptr;
rstl::reserved_vector<CDrawablePlaneObject, 8>* Buckets::sPlaneObjectData = nullptr;
rstl::reserved_vector<u16, 8>* Buckets::sPlaneObjectBucket = nullptr;
const float Buckets::skWorstMinMaxDistance[2] = {99999.f, -99999.f};
float Buckets::sMinMaxDistance[2];

void Buckets::Clear()
{
    sData->clear();
    sBucketIndex.clear();
    sPlaneObjectData->clear();
    sPlaneObjectBucket->clear();
    for (rstl::reserved_vector<CDrawable*, 128>& bucket : *sBuckets)
        bucket.clear();
    sMinMaxDistance[0] = skWorstMinMaxDistance[0];
    sMinMaxDistance[1] = skWorstMinMaxDistance[1];
}

void Buckets::Sort()
{
    float delta = std::max(1.f, sMinMaxDistance[1] - sMinMaxDistance[0]);
    sPlaneObjectBucket->resize(8);

    std::sort(sPlaneObjectBucket->begin(), sPlaneObjectBucket->end(),
    [](u16 a, u16 b) -> bool
    {
        return (*sPlaneObjectData)[a].GetDistance() >= (*sPlaneObjectData)[b].GetDistance();
    });

    u32 precision = 50 / (8 + 1);
    float pitch = 1.f / (delta / float(precision - 2));

    int accum = 0;
    for (u16 idx : *sPlaneObjectBucket)
    {
        ++accum;
        CDrawablePlaneObject& planeObj = (*sPlaneObjectData)[idx];
        planeObj.x24_targetBucket = precision * accum;
    }

    for (CDrawable& drawable : *sData)
    {
        int slot;
        if (sPlaneObjectBucket->empty())
        {
            slot = zeus::clamp(1, int((drawable.GetDistance() - sMinMaxDistance[0]) * pitch), 49);
        }
        else
        {
            /* TODO: Planar sort distribution */
        }

        if (slot == -1)
            slot = 49;
        (*sBuckets)[slot].push_back(&drawable);
    }

    int bucketIdx = sBuckets->size();
    for (auto it = sBuckets->rbegin() ; it != sBuckets->rend() ; ++it)
    {
        --bucketIdx;
        rstl::reserved_vector<CDrawable*, 128>& bucket = *it;
        if (bucket.size())
        {
            sBucketIndex.push_back(bucketIdx);
            std::sort(bucket.begin(), bucket.end(),
            [](CDrawable* a, CDrawable* b) -> bool
            {
                return a->GetDistance() >= b->GetDistance();
            });
        }
    }

    for (auto it = sPlaneObjectBucket->rbegin() ; it != sPlaneObjectBucket->rend() ; ++it)
    {
        CDrawablePlaneObject& planeObj = (*sPlaneObjectData)[*it];
        rstl::reserved_vector<CDrawable*, 128>& bucket = (*sBuckets)[planeObj.x24_targetBucket];
        bucket.push_back(&planeObj);
    }
}

void Buckets::InsertPlaneObject(float dist, float something, const zeus::CAABox& aabb, bool b1,
                                const zeus::CPlane& plane, bool b2, EDrawableType dtype, const void* data)
{
    sPlaneObjectData->push_back(CDrawablePlaneObject(dtype, dist, something, aabb, b1, plane, b2, data));
}

void Buckets::Insert(const zeus::CVector3f& pos, const zeus::CAABox& aabb, EDrawableType dtype,
                     const void* data, const zeus::CPlane& plane, u16 extraSort)
{
    float dist = plane.pointToPlaneDist(pos);
    sData->push_back(CDrawable(dtype, extraSort, dist, aabb, data));
    if (sMinMaxDistance[0] > dist)
        sMinMaxDistance[0] = dist;
    if (sMinMaxDistance[1] < dist)
        sMinMaxDistance[1] = dist;
}

void Buckets::Shutdown()
{
    sData = nullptr;
    sBuckets = nullptr;
    sPlaneObjectData = nullptr;
    sPlaneObjectBucket = nullptr;
}

void Buckets::Init()
{
    sData = &sDataHolder;
    sBuckets = &sBucketsHolder;
    sBuckets->resize(50);
    sPlaneObjectData = &sPlaneObjectDataHolder;
    sPlaneObjectBucket = &sPlaneObjectBucketHolder;
    sMinMaxDistance[0] = skWorstMinMaxDistance[0];
    sMinMaxDistance[1] = skWorstMinMaxDistance[1];
}

CBooRenderer::CAreaListItem::CAreaListItem
(const std::vector<CMetroidModelInstance>* geom, const CAreaRenderOctTree* octTree,
 std::vector<CBooModel*>&& models, int areaIdx)
: x0_geometry(geom), x4_octTree(octTree), x10_models(std::move(models)), x18_areaIdx(areaIdx) {}

CBooRenderer::CAreaListItem::~CAreaListItem() {}

void CBooRenderer::ActivateLightsForModel(CAreaListItem* item, CBooModel& model)
{
    std::vector<CLight> thisLights;
    thisLights.reserve(4);

    if (x304_lights.size())
    {
        /*
        void* unk3 = nullptr;
        int x14 = 0;
        if (item && item->x18_areaIdx != -1)
        {
            unk3 = item->x28_unk3;
            x14 = item->x4_octTree->x14_;
        }
        void* unk3Cur = unk3;
        */

        float lightRads[] = {-1.f, -1.f, -1.f, -1.f};
        for (int i=0 ; i<4 ; ++i)
        {
            for (CLight& light : x304_lights)
            {
                /*
                if (unk3)
                {
                }
                */
                if (light.x40_loadedIdx == i)
                    continue;
                float rad = light.GetRadius();
                if (model.x20_aabb.intersects(zeus::CSphere{light.x0_pos, rad}))
                {
                    if (rad > lightRads[i])
                    {
                        lightRads[i] = rad;
                        light.x40_loadedIdx = i;
                        thisLights.push_back(light);
                        break;
                    }
                }
            }
        }
    }

    //model.ActivateLights(thisLights);
}

void CBooRenderer::RenderBucketItems(CAreaListItem* item)
{
    CModelFlags flags;
    flags.m_extendedShaderIdx = 1;

    for (u16 idx : Buckets::sBucketIndex)
    {
        rstl::reserved_vector<CDrawable*, 128>& bucket = (*Buckets::sBuckets)[idx];
        for (CDrawable* drawable : bucket)
        {
            switch (drawable->GetType())
            {
            case EDrawableType::Particle:
            {
                static_cast<CParticleGen*>((void*)drawable->GetData())->Render();
                break;
            }
            case EDrawableType::WorldSurface:
            {
                CBooSurface* surf = static_cast<CBooSurface*>((void*)drawable->GetData());
                CBooModel* model = surf->m_parent;
                if (model)
                {
                    ActivateLightsForModel(item, *model);
                    model->DrawSurface(*surf, flags);
                }
                break;
            }
            default:
            {
                if (xa8_drawableCallback)
                {
                    xa8_drawableCallback(drawable->GetData(), xac_callbackContext,
                                       int(drawable->GetType()) - 2);
                }
                break;
            }
            }
        }
    }
}

void CBooRenderer::HandleUnsortedModel(CAreaListItem* item, CBooModel& model)
{
    ActivateLightsForModel(item, model);
    CBooSurface* surf = model.x38_firstUnsortedSurface;
    CModelFlags flags;
    flags.m_extendedShaderIdx = 1;
    while (surf)
    {
        model.DrawSurface(*surf, flags);
        surf = surf->m_next;
    }
}

void CBooRenderer::GenerateFogVolumeRampTex(boo::IGraphicsDataFactory::Context& ctx)
{
    u8 data[FOGVOL_RAMP_RES][FOGVOL_RAMP_RES] = {};
    for (int y=0 ; y<FOGVOL_RAMP_RES ; ++y)
    {
        for (int x=0 ; x<FOGVOL_RAMP_RES ; ++x)
        {
            int tmp = y << 16 | x << 8 | 0x7f;
            double a = zeus::clamp(0.0, (-150.0 / (tmp * 749.7998 - 750.0) - 0.2) * 3.0 / 749.7998, 1.0);
            data[y][x] = (a * a + a) / 2.f * 255;
        }
    }
    x1b8_fogVolumeRamp = ctx.newStaticTexture(FOGVOL_RAMP_RES, FOGVOL_RAMP_RES, 1,
                                              boo::TextureFormat::I8, data[0],
                                              FOGVOL_RAMP_RES * FOGVOL_RAMP_RES);
}

void CBooRenderer::GenerateSphereRampTex(boo::IGraphicsDataFactory::Context& ctx)
{
    u8 data[SPHERE_RAMP_RES][SPHERE_RAMP_RES] = {};
    float halfRes = SPHERE_RAMP_RES / 2.f;
    for (int y=0 ; y<SPHERE_RAMP_RES ; ++y)
    {
        for (int x=0 ; x<SPHERE_RAMP_RES ; ++x)
        {
            zeus::CVector2f vec((x - halfRes) / halfRes, (y - halfRes) / halfRes);
            data[y][x] = 255 - zeus::clamp(0.f, vec.canBeNormalized() ? vec.magnitude() : 0.f, 1.f) * 255;
        }
    }
    x220_sphereRamp = ctx.newStaticTexture(SPHERE_RAMP_RES, SPHERE_RAMP_RES, 1,
                                           boo::TextureFormat::I8, data[0],
                                           SPHERE_RAMP_RES * SPHERE_RAMP_RES);
}

void CBooRenderer::LoadThermoPalette()
{
    m_thermoPaletteTex = xc_store.GetObj("TXTR_ThermoPalette");
    CTexture* thermoTexObj = m_thermoPaletteTex.GetObj();
    if (thermoTexObj)
        x288_thermoPalette = thermoTexObj->GetPaletteTexture();
}

CBooRenderer::CBooRenderer(IObjectStore& store, IFactory& resFac)
: x8_factory(resFac), xc_store(store), x2a8_thermalRand(20)
{
    g_Renderer = this;
    xee_24_ = true;

    m_gfxToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        GenerateFogVolumeRampTex(ctx);
        GenerateSphereRampTex(ctx);
        return true;
    });
    LoadThermoPalette();
    m_thermHotFilter.emplace();

    Buckets::Init();
}

void CBooRenderer::AddWorldSurfaces(CBooModel& model)
{
    CBooSurface* surf = model.x3c_firstSortedSurface;
    while (surf)
    {
        const CBooModel::MaterialSet::Material& mat = model.GetMaterialByIndex(surf->m_data.matIdx);
        zeus::CAABox aabb = surf->GetBounds();
        zeus::CVector3f pt = aabb.closestPointAlongVector(xb0_viewPlane.vec);
        Buckets::Insert(pt, aabb, EDrawableType::WorldSurface, surf, xb0_viewPlane,
                        mat.heclIr.m_blendDst != boo::BlendFactor::Zero);
        surf = surf->m_next;
    }
}

std::list<CBooRenderer::CAreaListItem>::iterator
CBooRenderer::FindStaticGeometry(const std::vector<CMetroidModelInstance>* geometry)
{
    return std::find_if(x1c_areaListItems.begin(), x1c_areaListItems.end(),
                        [&](CAreaListItem& item) -> bool {return item.x0_geometry == geometry;});
}

void CBooRenderer::AddStaticGeometry(const std::vector<CMetroidModelInstance>* geometry,
                                     const CAreaRenderOctTree* octTree, int areaIdx)
{
    auto search = FindStaticGeometry(geometry);
    if (search == x1c_areaListItems.end())
    {
        std::vector<CBooModel*> models;
        if (geometry->size())
        {
            models.reserve(geometry->size());
            for (const CMetroidModelInstance& inst : *geometry)
                models.push_back(inst.m_instance);
        }
        x1c_areaListItems.emplace_back(geometry, octTree, std::move(models), areaIdx);
    }
}

void CBooRenderer::EnablePVS(const CPVSVisSet* set, u32 modelCount)
{
    xc8_pvs.emplace(*set);
    xe0_pvsModelCount = modelCount;
}

void CBooRenderer::DisablePVS()
{
    xc8_pvs = std::experimental::nullopt;
}

void CBooRenderer::RemoveStaticGeometry(const std::vector<CMetroidModelInstance>* geometry)
{
    auto search = FindStaticGeometry(geometry);
    if (search != x1c_areaListItems.end())
        x1c_areaListItems.erase(search);
}

void CBooRenderer::DrawUnsortedGeometry(int areaIdx, int mask, int targetMask)
{
    //SetupRendererStates(true);

    CAreaListItem* lastOctreeItem = nullptr;

    for (CAreaListItem& item : x1c_areaListItems)
    {
        if (areaIdx != -1 && item.x18_areaIdx != areaIdx)
            continue;

        if (item.x4_octTree)
            lastOctreeItem = &item;

        CPVSVisSet* pvs = nullptr;
        if (xc8_pvs)
            pvs = &*xc8_pvs;
        if (xe0_pvsModelCount != item.x10_models.size())
            pvs = nullptr;

        int idx = 0;
        for (auto it = item.x10_models.begin() ; it != item.x10_models.end() ; ++it, ++idx)
        {
            CBooModel* model = *it;
            if (pvs)
            {
                bool vis = pvs->GetVisible(idx);
                switch (xc4_pvsMode)
                {
                case EPVSMode::PVS:
                {
                    if (!vis)
                    {
                        model->x40_25_modelVisible = false;
                        continue;
                    }
                    break;
                }
                case EPVSMode::PVSAndMask:
                {
                    if (!vis && (model->x41_mask & mask) != targetMask)
                    {
                        model->x40_25_modelVisible = false;
                        continue;
                    }
                }
                default: break;
                }
            }

            if ((model->x41_mask & mask) != targetMask)
            {
                model->x40_25_modelVisible = false;
                continue;
            }

            if (!x44_frustumPlanes.aabbFrustumTest(model->x20_aabb))
            {
                model->x40_25_modelVisible = false;
                continue;
            }

            if (x318_25_drawWireframe)
            {
                model->x40_25_modelVisible = false;
                //HandleUnsortedModelWireframe();
                continue;
            }

            model->x40_25_modelVisible = true;
            HandleUnsortedModel(lastOctreeItem, *model);
        }

    }

    //SetupCGraphicsStates();
}

void CBooRenderer::DrawSortedGeometry(int areaIdx, int mask, int targetMask)
{
    //SetupRendererStates(true);

    CAreaListItem* lastOctreeItem = nullptr;

    for (CAreaListItem& item : x1c_areaListItems)
    {
        if (areaIdx != -1 && item.x18_areaIdx != areaIdx)
            continue;

        if (item.x4_octTree)
            lastOctreeItem = &item;

        for (auto it = item.x10_models.begin() ; it != item.x10_models.end() ; ++it)
        {
            CBooModel* model = *it;
            if (model->x40_25_modelVisible)
                AddWorldSurfaces(*model);
        }
    }

    Buckets::Sort();
    RenderBucketItems(lastOctreeItem);

    //SetupCGraphicsStates();
    //DrawRenderBucketsDebug();
    Buckets::Clear();
}

void CBooRenderer::DrawStaticGeometry(int modelCount, int mask, int targetMask)
{
    DrawUnsortedGeometry(modelCount, mask, targetMask);
    DrawSortedGeometry(modelCount, mask, targetMask);
}

void CBooRenderer::PostRenderFogs()
{
}

void CBooRenderer::AddParticleGen(const CParticleGen& gen)
{
    std::pair<zeus::CAABox, bool> bounds = gen.GetBounds();
    if (bounds.second)
    {
        zeus::CVector3f pt = bounds.first.closestPointAlongVector(xb0_viewPlane.vec);
        Buckets::Insert(pt, bounds.first, EDrawableType::Particle, &gen, xb0_viewPlane, 0);
    }
}

void CBooRenderer::AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int)
{

}

void CBooRenderer::AddDrawable(const void* obj, const zeus::CVector3f& pos, const zeus::CAABox& aabb, int mode, EDrawableSorting sorting)
{
    if (sorting == EDrawableSorting::UnsortedCallback)
        xa8_drawableCallback(obj, xac_callbackContext, mode);
    else
        Buckets::Insert(pos, aabb, EDrawableType(mode + 2), obj, xb0_viewPlane, 0);
}

void CBooRenderer::SetDrawableCallback(TDrawableCallback&& cb, const void* ctx)
{
    xa8_drawableCallback = std::move(cb);
    xac_callbackContext = ctx;
}

void CBooRenderer::SetWorldViewpoint(const zeus::CTransform& xf)
{
    CGraphics::SetViewPointMatrix(xf);
    xb0_viewPlane.vec = xf.basis[1];
    xb0_viewPlane.d = xf.basis[1].dot(xf.origin);
}

void CBooRenderer::SetPerspective(float fovy, float width, float height, float znear, float zfar)
{
    CGraphics::SetPerspective(fovy, width / height, znear, zfar);
}

void CBooRenderer::SetPerspective(float fovy, float aspect, float znear, float zfar)
{
    CGraphics::SetPerspective(fovy, aspect, znear, zfar);
}

void CBooRenderer::SetViewportOrtho(bool centered, float znear, float zfar)
{
    float left = centered ? CGraphics::g_ViewportResolutionHalf.x : 0;
    float bottom = centered ? CGraphics::g_ViewportResolutionHalf.y : 0;
    float top = centered ? CGraphics::g_ViewportResolutionHalf.y : CGraphics::g_ViewportResolution.y;
    float right = centered ? CGraphics::g_ViewportResolutionHalf.x : CGraphics::g_ViewportResolution.x;

    CGraphics::SetOrtho(left, right, top, bottom, znear, zfar);
    CGraphics::SetViewPointMatrix(zeus::CTransform::Identity());
    CGraphics::SetModelMatrix(zeus::CTransform::Identity());
}

void CBooRenderer::SetClippingPlanes(const zeus::CFrustum& frustum)
{
    x44_frustumPlanes = frustum;
}

void CBooRenderer::SetViewport(int l, int b, int w, int h)
{
    CGraphics::SetViewport(l, b, w, h);
    CGraphics::SetScissor(l, b, w, h);
}

void CBooRenderer::SetDebugOption(EDebugOption, int)
{
}

void CBooRenderer::BeginScene()
{
    CGraphics::SetViewport(0, 0, CGraphics::g_ViewportResolution.x, CGraphics::g_ViewportResolution.y);
    CGraphics::SetPerspective(75.f, CGraphics::g_ProjAspect, 1.f, 4096.f);
    CGraphics::SetModelMatrix(zeus::CTransform::Identity());
    CGraphics::BeginScene();
}

void CBooRenderer::EndScene()
{
    CGraphics::EndScene();
}

void CBooRenderer::SetAmbientColor(const zeus::CColor& color)
{
    CGraphics::SetAmbientColor(color);
}

void CBooRenderer::DrawString(const char*, int, int)
{
}

u32 CBooRenderer::GetFPS()
{
    return 0;
}

void CBooRenderer::DrawSpaceWarp(const zeus::CVector3f& pt, float strength)
{
    m_spaceWarpFilter.setStrength(strength);
    m_spaceWarpFilter.draw(pt);
}

void CBooRenderer::DrawThermalModel(const CModel& model, const zeus::CColor& mulCol, const zeus::CColor& addCol)
{
    CModelFlags flags;
    flags.m_extendedShaderIdx = 2;
    flags.color = mulCol;
    flags.addColor = addCol;
    model.Draw(flags);
}

void CBooRenderer::DrawXRayOutline(const zeus::CAABox& aabb)
{
    CModelFlags flags;
    flags.m_extendedShaderIdx = 3;

    for (CAreaListItem& item : x1c_areaListItems)
    {
        if (item.x4_octTree)
        {
            std::vector<u32> bitmap;
            item.x4_octTree->FindOverlappingModels(bitmap, aabb);

            for (u32 c=0 ; c<item.x4_octTree->x14_bitmapWordCount ; ++c)
            {
                for (u32 b=0 ; b<32 ; ++b)
                {
                    if (bitmap[c] & (1 << b))
                    {
                        CBooModel* model = item.x10_models[c * 32 + b];
                        model->UpdateUniformData(flags);
                        const CBooSurface* surf = model->x38_firstUnsortedSurface;
                        while (surf)
                        {
                            if (surf->GetBounds().intersects(aabb))
                                model->DrawSurface(*surf, flags);
                            surf = surf->m_next;
                        }
                    }
                }
            }
        }
    }
}

void CBooRenderer::SetWireframeFlags(int)
{
}

void CBooRenderer::SetWorldFog(ERglFogMode mode, float startz, float endz, const zeus::CColor& color)
{
    if (x318_28_disableFog)
        mode = ERglFogMode::None;
    CGraphics::SetFog(mode, startz, endz, color);
}

void CBooRenderer::RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*)
{
}

void CBooRenderer::SetThermal(bool thermal, float level, const zeus::CColor& color)
{
    x318_29_thermalVisor = thermal;
    x2f0_thermalVisorLevel = level;
    x2f4_thermColor = color;
}

void CBooRenderer::SetThermalColdScale(float scale)
{
    x2f8_thermColdScale = zeus::clamp(0.f, scale, 1.f);
}

void CBooRenderer::DoThermalBlendCold()
{    
    zeus::CColor a = zeus::CColor::lerp(x2f4_thermColor, zeus::CColor::skWhite, x2f8_thermColdScale);
    m_thermColdFilter.setColorA(a);
    float bFac = 0.f;
    float bAlpha = 1.f;
    if (x2f8_thermColdScale < 0.5f)
    {
        bAlpha = x2f8_thermColdScale * 2.f;
        bFac = (1.f - bAlpha) / 8.f;
    }
    zeus::CColor b{bFac, bFac, bFac, bAlpha};
    m_thermColdFilter.setColorB(b);
    zeus::CColor c = zeus::CColor::lerp(zeus::CColor::skBlack, zeus::CColor::skWhite,
                                        zeus::clamp(0.f, (x2f8_thermColdScale - 0.25f) * 4.f / 3.f, 1.f));
    m_thermColdFilter.setColorC(c);

    m_thermColdFilter.setScale(x2f8_thermColdScale);

    m_thermColdFilter.setShift(x2a8_thermalRand.Next() % 32);
    m_thermColdFilter.draw();
}

void CBooRenderer::DoThermalBlendHot()
{
    m_thermHotFilter->draw();
}

u32 CBooRenderer::GetStaticWorldDataSize()
{
    return 0;
}

}
