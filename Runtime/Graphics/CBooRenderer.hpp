#ifndef __URDE_CBOORENDERER_HPP__
#define __URDE_CBOORENDERER_HPP__

#include <functional>
#include "IRenderer.hpp"
#include "CDrawable.hpp"
#include "CDrawablePlaneObject.hpp"
#include "Shaders/CThermalColdFilter.hpp"
#include "Shaders/CThermalHotFilter.hpp"
#include "Shaders/CSpaceWarpFilter.hpp"
#include "CRandom16.hpp"
#include "CPVSVisSet.hpp"
#include "zeus/CRectangle.hpp"
#include "World/CGameArea.hpp"
#include "zeus/CPlane.hpp"

namespace urde
{
class IObjectStore;
class CMemorySys;
class IFactory;
class CTexture;
class CParticleGen;
class CBooModel;

class Buckets
{
    friend class CBooRenderer;

    static rstl::reserved_vector<u16, 50> sBucketIndex;
    static rstl::reserved_vector<CDrawable, 50>* sData;
    static rstl::reserved_vector<rstl::reserved_vector<CDrawable*, 128>, 50>* sBuckets;
    static rstl::reserved_vector<CDrawablePlaneObject, 8>* sPlaneObjectData;
    static rstl::reserved_vector<u16, 8>* sPlaneObjectBucket;
    static const float skWorstMinMaxDistance[2];
    static float sMinMaxDistance[2];

public:
    static void Clear();
    static void Sort();
    static void InsertPlaneObject(float dist, float something, const zeus::CAABox& aabb, bool b1,
                                  const zeus::CPlane& plane, bool b2, EDrawableType dtype, const void* data);
    static void Insert(const zeus::CVector3f& pos, const zeus::CAABox& aabb, EDrawableType dtype,
                       const void* data, const zeus::CPlane& plane, u16 extraSort);
    static void Shutdown();
    static void Init();
};

class CBooRenderer : public IRenderer
{
    friend class CBooModel;
    friend class CWorldTransManager;
    friend class CMorphBallShadow;

    struct CAreaListItem
    {
        const std::vector<CMetroidModelInstance>* x0_geometry;
        const CAreaRenderOctTree* x4_octTree;
        /* originally auto_ptrs of vectors */
        std::vector<TCachedToken<CTexture>> x8_textures;
        std::vector<CBooModel*> x10_models;
        int x18_areaIdx;
        /* Per-area octree-word major, light bits minor */
        std::vector<u32> x1c_lightOctreeWords;

        CAreaListItem(const std::vector<CMetroidModelInstance>* geom,
                      const CAreaRenderOctTree* octTree,
                      std::vector<TCachedToken<CTexture>>&& textures,
                      std::vector<CBooModel*>&& models, int areaIdx);
        ~CAreaListItem();
    };

    struct CFogVolumeListItem
    {
        zeus::CTransform x0_transform;
        zeus::CColor x30_color;
        zeus::CAABox x34_aabb;
        TLockedToken<CModel> x4c_model;
        //bool x58_b; Optional for model token
        const CSkinnedModel* x5c_skinnedModel = nullptr;
        CFogVolumeListItem(const zeus::CTransform& xf, const zeus::CColor& color,
                           const zeus::CAABox& aabb, const TLockedToken<CModel>* model,
                           const CSkinnedModel* sModel)
        : x0_transform(xf), x30_color(color), x34_aabb(aabb), x5c_skinnedModel(sModel)
        {
            if (model)
                x4c_model = *model;
        }
    };

    IFactory& x8_factory;
    IObjectStore& xc_store;
    boo::GraphicsDataToken m_gfxToken;
    // CFont x10_fnt;
    u32 x18_ = 0;
    std::list<CAreaListItem> x1c_areaListItems;
    zeus::CFrustum x44_frustumPlanes;

    TDrawableCallback xa8_drawableCallback;
    const void* xac_callbackContext;

    zeus::CPlane xb0_viewPlane = {0.f, 1.f, 0.f, 0.f};

    enum class EPVSMode
    {
        Mask,
        PVS,
        PVSAndMask
    } xc4_pvsMode = EPVSMode::Mask;
    std::experimental::optional<CPVSVisSet> xc8_pvs;
    u32 xe0_pvsAreaIdx = 0;

    //boo::ITextureS* xe4_blackTex = nullptr;
    bool xee_24_ : 1;

    boo::ITextureR* x14c_reflectionTex = nullptr;
    //boo::ITextureS* x150_mirrorRamp = nullptr;
    boo::ITextureS* x1b8_fogVolumeRamp = nullptr;
    boo::ITextureS* x220_sphereRamp = nullptr;
    TLockedToken<CTexture> m_thermoPaletteTex;
    boo::ITexture* x288_thermoPalette = nullptr;
    TLockedToken<CTexture> m_ballFadeTex;
    boo::ITexture* m_ballFade = nullptr;
    boo::ITextureR* m_ballShadowId = nullptr;
    int m_ballShadowIdW = 64;
    int m_ballShadowIdH = 64;

    CRandom16 x2a8_thermalRand;
    std::list<CFogVolumeListItem> x2ac_fogVolumes;
    std::list<std::pair<zeus::CVector3f, float>> x2c4_spaceWarps;
    zeus::CColor x2e0_ = zeus::CColor::skWhite;
    zeus::CVector3f x2e4_ = {0.f, 1.f, 0.f};

    CSpaceWarpFilter m_spaceWarpFilter;

    float x2f0_thermalVisorLevel;
    zeus::CColor x2f4_thermColor;
    float x2f8_thermColdScale = 0.f;
    zeus::CColor x2fc_tevReg1Color = {1.f, 0.f, 1.f, 1.f};
    CThermalColdFilter m_thermColdFilter;
    std::experimental::optional<CThermalHotFilter> m_thermHotFilter;

    std::vector<CLight> x300_dynamicLights;

    union
    {
        struct
        {
            bool x318_24_refectionDirty : 1;
            bool x318_25_drawWireframe : 1;
            bool x318_26_requestRGBA6 : 1;
            bool x318_27_currentRGBA6 : 1;
            bool x318_28_disableFog : 1;
            bool x318_29_thermalVisor : 1;
            bool x318_30_inAreaDraw : 1;
            bool x318_31_persistRGBA6 : 1;
        };
        u16 dummy = 0;
    };

    void GenerateFogVolumeRampTex(boo::IGraphicsDataFactory::Context& ctx);
    void GenerateSphereRampTex(boo::IGraphicsDataFactory::Context& ctx);
    void LoadThermoPalette();
    void LoadBallFade();

    void ActivateLightsForModel(CAreaListItem* item, CBooModel& model);
    void RenderBucketItems(CAreaListItem* item);
    void HandleUnsortedModel(CAreaListItem* item, CBooModel& model);
    static void DrawFogSlices(const zeus::CPlane* planes, int numPlanes, int iteration,
                              const zeus::CVector3f& center, float delta);
    static void RenderFogVolumeModel(const zeus::CAABox& aabb, const CModel* model, const zeus::CTransform& modelMtx,
                                     const zeus::CTransform& viewMtx, const CSkinnedModel* sModel);
    void ReallyRenderFogVolume(const zeus::CColor& color, const zeus::CAABox& aabb,
                               const CModel* model, const CSkinnedModel* sModel);

public:
    CBooRenderer(IObjectStore& store, IFactory& resFac);

    void AddWorldSurfaces(CBooModel& model);

    std::list<CAreaListItem>::iterator FindStaticGeometry(const std::vector<CMetroidModelInstance>*);
    void AddStaticGeometry(const std::vector<CMetroidModelInstance>*, const CAreaRenderOctTree*, int areaIdx);
    void EnablePVS(const CPVSVisSet*, u32);
    void DisablePVS();
    void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>*);
    void DrawAreaGeometry(int areaIdx, int mask, int targetMask);
    void DrawUnsortedGeometry(int areaIdx, int mask, int targetMask);
    void DrawSortedGeometry(int areaIdx, int mask, int targetMask);
    void DrawStaticGeometry(int areaIdx, int mask, int targetMask);
    void PostRenderFogs();
    void AddParticleGen(const CParticleGen&);
    void AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int);
    void AddDrawable(void const *, const zeus::CVector3f&, const zeus::CAABox&, int, EDrawableSorting);
    void SetDrawableCallback(TDrawableCallback, const void*);
    void SetWorldViewpoint(const zeus::CTransform&);
    void SetPerspective(float, float, float, float, float);
    void SetPerspective(float, float, float, float);
    zeus::CRectangle SetViewportOrtho(bool, float, float);
    void SetClippingPlanes(const zeus::CFrustum& frustum);
    void SetViewport(int, int, int, int);
    //void SetDepthReadWrite(bool, bool);
    //void SetBlendMode_AdditiveAlpha();
    //void SetBlendMode_AlphaBlended();
    //void SetBlendMode_NoColorWrite();
    //void SetBlendMode_ColorMultiply();
    //void SetBlendMode_InvertDst();
    //void SetBlendMode_InvertSrc();
    //void SetBlendMode_Replace();
    //void SetBlendMode_AdditiveDestColor();
    void SetDebugOption(EDebugOption, int);
    void BeginScene();
    void EndScene();
    //void BeginPrimitive(EPrimitiveType, int);
    //void BeginLines(int);
    //void BeginLineStrip(int);
    //void BeginTriangles(int);
    //void BeginTriangleStrip(int);
    //void BeginTriangleFan(int);
    //void PrimVertex(const zeus::CVector3f&);
    //void PrimNormal(const zeus::CVector3f&);
    //void PrimColor(float, float, float, float);
    //void PrimColor(const zeus::CColor&);
    //void EndPrimitive();
    void SetAmbientColor(const zeus::CColor&);
    void DrawString(const char*, int, int);
    u32 GetFPS();
    //void CacheReflection(TReflectionCallback, void*, bool);
    void DrawSpaceWarp(const zeus::CVector3f&, float);
    void DrawThermalModel(const CModel& model, const zeus::CColor& multCol, const zeus::CColor& addCol);
    void DrawXRayOutline(const zeus::CAABox&);
    void SetWireframeFlags(int);
    void SetWorldFog(ERglFogMode, float, float, const zeus::CColor&);
    void RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*);
    void SetThermal(bool, float, const zeus::CColor&);
    void SetThermalColdScale(float scale);
    void DoThermalBlendCold();
    void DoThermalBlendHot();
    u32 GetStaticWorldDataSize();
    void PrepareDynamicLights(const std::vector<CLight>& lights);
    void SetWorldLightFadeLevel(float level);

    boo::ITexture* GetThermoPalette() {return x288_thermoPalette;}

    void BindMainDrawTarget() {CGraphics::g_BooMainCommandQueue->setRenderTarget(CGraphics::g_SpareTexture);}
    void BindReflectionDrawTarget() {CGraphics::g_BooMainCommandQueue->setRenderTarget(x14c_reflectionTex);}
    void BindBallShadowIdTarget()
    {
        CGraphics::g_BooMainCommandQueue->setRenderTarget(m_ballShadowId);
        SetViewport(0, 0, m_ballShadowIdW, m_ballShadowIdH);
    }
    void ResolveBallShadowIdTarget()
    {
        CGraphics::g_BooMainCommandQueue->resolveBindTexture(m_ballShadowId,
                                                             boo::SWindowRect(0, 0,
                                                                              m_ballShadowIdW,
                                                                              m_ballShadowIdH),
                                                             false, true, false);
    }

    void FindOverlappingWorldModels(std::vector<u32>& modelBits, const zeus::CAABox& aabb) const;
    int DrawOverlappingWorldModelIDs(int alphaVal, const std::vector<u32>& modelBits,
                                     const zeus::CAABox& aabb) const;
    void DrawOverlappingWorldModelShadows(int alphaVal, const std::vector<u32>& modelBits,
                                          const zeus::CAABox& aabb, float alpha) const;
};

}

#endif // __URDE_CBOORENDERER_HPP__
