#if _WIN32
#include <D3Dcommon.h>
#endif
#include "GameGlobalObjects.hpp"
#include "CBooRenderer.hpp"
#include "CTexture.hpp"
#include "CModel.hpp"
#include "Particle/CParticleGen.hpp"
#include "CMetroidModelInstance.hpp"

#define MIRROR_RAMP_RES 32
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
        sBucketIndex.push_back(bucketIdx);
        rstl::reserved_vector<CDrawable*, 128>& bucket = *it;
        if (bucket.size())
        {
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
(const std::vector<CMetroidModelInstance>* geom, const CAreaOctTree* octTree,
 std::vector<CBooModel*>&& models, int unk)
: x0_geometry(geom), x4_octTree(octTree), x10_models(std::move(models)), x18_unk(unk) {}

CBooRenderer::CAreaListItem::~CAreaListItem() {}

void CBooRenderer::RenderBucketItems(const std::vector<CLight>& lights)
{
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
            case EDrawableType::Surface:
            {
                CBooSurface* surf = static_cast<CBooSurface*>((void*)drawable->GetData());
                CBooModel* model = surf->m_parent;
                if (model)
                {
                    model->ActivateLights(lights);
                    model->DrawSurface(*surf, CModelFlags{});
                }
                break;
            }
            default:
            {
                if (xa8_renderCallback)
                {
                    xa8_renderCallback(drawable->GetData(), xac_callbackContext,
                                       int(drawable->GetType()) - 2);
                }
                break;
            }
            }
        }
    }
}

void CBooRenderer::GenerateMirrorRampTex(boo::IGraphicsDataFactory::Context& ctx)
{
    u8 data[MIRROR_RAMP_RES][MIRROR_RAMP_RES][4] = {};
    float halfRes = MIRROR_RAMP_RES / 2.f;
    for (int y=0 ; y<MIRROR_RAMP_RES ; ++y)
    {
        for (int x=0 ; x<MIRROR_RAMP_RES ; ++x)
        {
            zeus::CVector2f vec((x - halfRes) / halfRes, (y - halfRes) / halfRes);
            if (vec.magnitude() <= halfRes && vec.canBeNormalized())
                vec.normalize();
            data[y][x][0] = zeus::clamp(0.f, vec.x, 1.f) * 255;
            data[y][x][1] = zeus::clamp(0.f, vec.y, 1.f) * 255;
        }
    }
    x150_mirrorRamp = ctx.newStaticTexture(MIRROR_RAMP_RES, MIRROR_RAMP_RES, 1,
                                           boo::TextureFormat::RGBA8, data[0],
                                           MIRROR_RAMP_RES * MIRROR_RAMP_RES * 4);
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
            data[y][x] = zeus::clamp(0.f, vec.canBeNormalized() ? vec.magnitude() : 0.f, 1.f) * 255;
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
        x288_thermoPalette = thermoTexObj->GetBooTexture();
}

CBooRenderer::CBooRenderer(IObjectStore& store, IFactory& resFac)
: x8_factory(resFac), xc_store(store), x2a8_thermalRand(20)
{
    xee_24_ = true;

    m_gfxToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        GenerateMirrorRampTex(ctx);
        GenerateFogVolumeRampTex(ctx);
        GenerateSphereRampTex(ctx);
        return true;
    });
    LoadThermoPalette();

    Buckets::Init();
}

void CBooRenderer::AddWorldSurfaces(CBooModel& model)
{
    CBooSurface* surf = model.x3c_firstSortedSurface;
    while (surf)
    {
        const CBooModel::MaterialSet::Material& mat = model.GetMaterialByIndex(surf->selfIdx);
        zeus::CAABox aabb = surf->GetBounds();
        zeus::CVector3f pt = aabb.closestPointAlongVector(xb0_.vec);
        Buckets::Insert(pt, aabb, EDrawableType::Surface, surf, xb0_,
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
                                     const CAreaOctTree* octTree, int unk)
{
    auto search = FindStaticGeometry(geometry);
    if (search == x1c_areaListItems.end())
    {
        std::vector<CBooModel*> models;
        if (geometry->size())
        {
            models.reserve(geometry->size());
            for (const CMetroidModelInstance& inst : *geometry)
                models.push_back(inst.m_instance.get());
        }
        x1c_areaListItems.emplace_back(geometry, octTree, std::move(models), unk);
    }
}

void CBooRenderer::RemoveStaticGeometry(const std::vector<CMetroidModelInstance>* geometry)
{
    auto search = FindStaticGeometry(geometry);
    if (search != x1c_areaListItems.end())
        x1c_areaListItems.erase(search);
}

void CBooRenderer::DrawUnsortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)
{
}

void CBooRenderer::DrawSortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)
{
    //SetupRendererStates();
}

void CBooRenderer::DrawStaticGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int)
{
}

void CBooRenderer::PostRenderFogs()
{
}

void CBooRenderer::AddParticleGen(const CParticleGen& gen)
{
    std::pair<zeus::CAABox, bool> bounds = gen.GetBounds();
    if (bounds.second)
    {
        zeus::CVector3f pt = bounds.first.closestPointAlongVector(xb0_.vec);
        Buckets::Insert(pt, bounds.first, EDrawableType::Particle, &gen, xb0_, 0);
    }
}

void CBooRenderer::AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int)
{

}

void CBooRenderer::AddDrawable(const void* obj, const zeus::CVector3f& pos, const zeus::CAABox& aabb, int mode, EDrawableSorting sorting)
{
    if (sorting == EDrawableSorting::UnsortedCallback)
        xa8_renderCallback(obj, xac_callbackContext, mode);
    else
        Buckets::Insert(pos, aabb, EDrawableType(mode + 2), obj, xb0_, 0);
}

void CBooRenderer::SetDrawableCallback(TDrawableCallback&& cb, const void* ctx)
{
    xa8_renderCallback = std::move(cb);
    xac_callbackContext = ctx;
}

void CBooRenderer::SetWorldViewpoint(const zeus::CTransform&)
{
}

void CBooRenderer::SetPerspectiveFovScalar(float)
{
}

void CBooRenderer::SetPerspective(float, float, float, float, float)
{
}

void CBooRenderer::SetPerspective(float, float, float, float)
{
}

void CBooRenderer::SetViewportOrtho(bool, float, float)
{
}

void CBooRenderer::SetClippingPlanes(const zeus::CFrustum& frustum)
{
    x44_frustumPlanes = frustum;
}

void CBooRenderer::SetViewport(int, int, int, int)
{
}

void CBooRenderer::SetDepthReadWrite(bool, bool)
{
}

void CBooRenderer::SetBlendMode_AdditiveAlpha()
{
}

void CBooRenderer::SetBlendMode_AlphaBlended()
{
}

void CBooRenderer::SetBlendMode_NoColorWrite()
{
}

void CBooRenderer::SetBlendMode_ColorMultiply()
{
}

void CBooRenderer::SetBlendMode_InvertDst()
{
}

void CBooRenderer::SetBlendMode_InvertSrc()
{
}

void CBooRenderer::SetBlendMode_Replace()
{
}

void CBooRenderer::SetBlendMode_AdditiveDestColor()
{
}

void CBooRenderer::SetDebugOption(EDebugOption, int)
{
}

void CBooRenderer::BeginScene()
{
}

void CBooRenderer::EndScene()
{
}

void CBooRenderer::BeginPrimitive(EPrimitiveType, int)
{
}

void CBooRenderer::BeginLines(int)
{
}

void CBooRenderer::BeginLineStrip(int)
{
}

void CBooRenderer::BeginTriangles(int)
{
}

void CBooRenderer::BeginTriangleStrip(int)
{
}

void CBooRenderer::BeginTriangleFan(int)
{
}

void CBooRenderer::PrimVertex(const zeus::CVector3f&)
{
}

void CBooRenderer::PrimNormal(const zeus::CVector3f&)
{
}

void CBooRenderer::PrimColor(float, float, float, float)
{
}

void CBooRenderer::PrimColor(const zeus::CColor&)
{
}

void CBooRenderer::EndPrimitive()
{
}

void CBooRenderer::SetAmbientColor(const zeus::CColor&)
{
}

void CBooRenderer::SetStaticWorldAmbientColor(const zeus::CColor&)
{
}

void CBooRenderer::DrawString(const char*, int, int)
{
}

u32 CBooRenderer::GetFPS()
{
    return 0;
}

void CBooRenderer::CacheReflection(TReflectionCallback, void*, bool)
{
}

void CBooRenderer::DrawSpaceWarp(const zeus::CVector3f&, float)
{
}

void CBooRenderer::DrawThermalModel(const CModel&, const zeus::CColor&, const zeus::CColor&, const float*, const float*)
{
}

void CBooRenderer::DrawXRayOutline(const CModel&, const float*, const float*)
{
}

void CBooRenderer::SetWireframeFlags(int)
{
}

void CBooRenderer::SetWorldFog(ERglFogMode, float, float, const zeus::CColor&)
{
}

void CBooRenderer::RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*)
{
}

void CBooRenderer::SetThermal(bool, float, const zeus::CColor&)
{
}

void CBooRenderer::SetThermalColdScale(float scale)
{
    x2f8_thermColdScale = zeus::clamp(0.f, scale, 1.f);
    m_thermColdFilter.setScale(x2f8_thermColdScale);
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
                                        (x2f8_thermColdScale - 0.25f) * 4.f / 3.f);
    m_thermColdFilter.setColorC(c);

    m_thermColdFilter.setScale(x2f8_thermColdScale);

    m_thermColdFilter.setShift(x2a8_thermalRand.Next() % 32);
    m_thermColdFilter.draw();
}

void CBooRenderer::DoThermalBlendHot()
{
}

u32 CBooRenderer::GetStaticWorldDataSize()
{
    return 0;
}

}
