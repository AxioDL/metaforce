#ifndef __URDE_CBOORENDERER_HPP__
#define __URDE_CBOORENDERER_HPP__

#include <functional>
#include "IRenderer.hpp"
#include "CDrawable.hpp"
#include "CDrawablePlaneObject.hpp"
#include "Shaders/CThermalColdFilter.hpp"
#include "CRandom16.hpp"

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
    struct CAreaListItem
    {
        const std::vector<CMetroidModelInstance>* x0_geometry;
        const CAreaOctTree* x4_octTree;
        //std::vector<TCachedToken<CTexture>> x8_textures;
        std::vector<CBooModel*> x10_models;
        int x18_unk;
        int x20_unk1 = 0;
        int x24_unk2 = 0;
        int x28_unk3 = 0;

        CAreaListItem(const std::vector<CMetroidModelInstance>* geom, const CAreaOctTree* octTree,
                      std::vector<CBooModel*>&& models, int unk);
        ~CAreaListItem();
    };

    IFactory& x8_factory;
    IObjectStore& xc_store;
    boo::GraphicsDataToken m_gfxToken;
    // CFont x10_fnt;
    u32 x18_ = 0;
    std::list<CAreaListItem> x1c_areaListItems;
    zeus::CFrustum x44_frustumPlanes;

    TDrawableCallback xa8_renderCallback;
    const void* xac_callbackContext;

    zeus::CPlane xb0_ = {0.f, 1.f, 0.f, 0.f};

    //boo::ITextureS* xe4_blackTex = nullptr;
    bool xee_24_ : 1;

    boo::ITextureR* x14c_reflectionTex = nullptr;
    boo::ITextureS* x150_mirrorRamp = nullptr;
    boo::ITextureS* x1b8_fogVolumeRamp = nullptr;
    boo::ITextureS* x220_sphereRamp = nullptr;
    TLockedToken<CTexture> m_thermoPaletteTex;
    boo::ITexture* x288_thermoPalette = nullptr;

    CRandom16 x2a8_thermalRand;
    std::list<u32> x2b8_;
    std::list<u32> x2d0_;
    zeus::CColor x2e0_ = zeus::CColor::skWhite;
    zeus::CVector3f x2e4_ = {0.f, 1.f, 0.f};

    zeus::CColor x2f4_thermColor;
    float x2f8_thermColdScale = 0.f;
    CThermalColdFilter m_thermColdFilter;

    union
    {
        struct
        {
            bool x318_24_refectionDirty : 1;
            bool x318_25_ : 1;
            bool x318_26_ : 1;
            bool x318_27_ : 1;
            bool x318_28_ : 1;
            bool x318_29_ : 1;
            bool x318_30_ : 1;
            bool x318_31_ : 1;
        };
        u16 dummy = 0;
    };

    void GenerateMirrorRampTex(boo::IGraphicsDataFactory::Context& ctx);
    void GenerateFogVolumeRampTex(boo::IGraphicsDataFactory::Context& ctx);
    void GenerateSphereRampTex(boo::IGraphicsDataFactory::Context& ctx);
    void LoadThermoPalette();

    void RenderBucketItems(const std::vector<CLight>& lights);

public:
    CBooRenderer(IObjectStore& store, IFactory& resFac);

    void AddWorldSurfaces(CBooModel& model);

    std::list<CAreaListItem>::iterator FindStaticGeometry(const std::vector<CMetroidModelInstance>*);
    void AddStaticGeometry(const std::vector<CMetroidModelInstance>*, const CAreaOctTree*, int);
    void RemoveStaticGeometry(const std::vector<CMetroidModelInstance>*);
    void DrawUnsortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    void DrawSortedGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    void DrawStaticGeometry(const std::vector<CLight>&, int, unsigned int, unsigned int);
    void PostRenderFogs();
    void AddParticleGen(const CParticleGen&);
    void AddPlaneObject(const void*, const zeus::CAABox&, const zeus::CPlane&, int);
    void AddDrawable(void const *, const zeus::CVector3f&, const zeus::CAABox&, int, EDrawableSorting);
    void SetDrawableCallback(TDrawableCallback&&, const void*);
    void SetWorldViewpoint(const zeus::CTransform&);
    void SetPerspectiveFovScalar(float);
    void SetPerspective(float, float, float, float, float);
    void SetPerspective(float, float, float, float);
    void SetViewportOrtho(bool, float, float);
    void SetClippingPlanes(const zeus::CFrustum& frustum);
    void SetViewport(int, int, int, int);
    void SetDepthReadWrite(bool, bool);
    void SetBlendMode_AdditiveAlpha();
    void SetBlendMode_AlphaBlended();
    void SetBlendMode_NoColorWrite();
    void SetBlendMode_ColorMultiply();
    void SetBlendMode_InvertDst();
    void SetBlendMode_InvertSrc();
    void SetBlendMode_Replace();
    void SetBlendMode_AdditiveDestColor();
    void SetDebugOption(EDebugOption, int);
    void BeginScene();
    void EndScene();
    void BeginPrimitive(EPrimitiveType, int);
    void BeginLines(int);
    void BeginLineStrip(int);
    void BeginTriangles(int);
    void BeginTriangleStrip(int);
    void BeginTriangleFan(int);
    void PrimVertex(const zeus::CVector3f&);
    void PrimNormal(const zeus::CVector3f&);
    void PrimColor(float, float, float, float);
    void PrimColor(const zeus::CColor&);
    void EndPrimitive();
    void SetAmbientColor(const zeus::CColor&);
    void SetStaticWorldAmbientColor(const zeus::CColor&);
    void DrawString(const char*, int, int);
    u32 GetFPS();
    void CacheReflection(TReflectionCallback, void*, bool);
    void DrawSpaceWarp(const zeus::CVector3f&, float);
    void DrawThermalModel(const CModel&, const zeus::CColor&, const zeus::CColor&, const float*, const float*);
    void DrawXRayOutline(const CModel&, const float*, const float*);
    void SetWireframeFlags(int);
    void SetWorldFog(ERglFogMode, float, float, const zeus::CColor&);
    void RenderFogVolume(const zeus::CColor&, const zeus::CAABox&, const TLockedToken<CModel>*, const CSkinnedModel*);
    void SetThermal(bool, float, const zeus::CColor&);
    void SetThermalColdScale(float scale);
    void DoThermalBlendCold();
    void DoThermalBlendHot();
    u32 GetStaticWorldDataSize();

    void BindMainDrawTarget() {CGraphics::g_BooMainCommandQueue->setRenderTarget(CGraphics::g_SpareTexture);}
    void BindReflectionDrawTarget() {CGraphics::g_BooMainCommandQueue->setRenderTarget(x14c_reflectionTex);}
};

}

#endif // __URDE_CBOORENDERER_HPP__
