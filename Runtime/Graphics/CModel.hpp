#ifndef __URDE_CMODEL_HPP__
#define __URDE_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "CFactoryMgr.hpp"
#include "CToken.hpp"
#include "zeus/CAABox.hpp"
#include "DNACommon/CMDL.hpp"
#include "DNAMP1/CMDLMaterials.hpp"
#include "Shaders/CModelShaders.hpp"

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{
class IObjectStore;
class CTexture;
class CLight;
class CSkinRules;
class CPoseAsTransforms;
class CModel;

struct CModelFlags
{
    u8 m_blendMode = 0; /* Blend state 3/5 enable additive */
    u8 m_matSetIdx = 0;
    u8 m_extendedShaderIdx = 0; /* 0 for shadeless, 1 for lighting, others defined in CModelShaders */
    u16 m_flags = 0; /* Flags */
    zeus::CColor color; /* Set into kcolor slot specified by material */
    zeus::CColor addColor = zeus::CColor::skClear;
    zeus::CColor regColors[3];

    CModelFlags() = default;
    CModelFlags(u8 blendMode, u8 shadIdx, u16 flags, const zeus::CColor& col)
    : m_blendMode(blendMode), m_matSetIdx(shadIdx), m_flags(flags), color(col) {}

    /* Flags
        0x4: render without texture lock
        0x8: depth greater
        0x10: depth non-inclusive
     */
};

/* urde addition: doesn't require hacky stashing of
 * pointers within loaded CMDL buffer */
struct CBooSurface
{
    DataSpec::DNACMDL::SurfaceHeader_2 m_data;
    size_t selfIdx;
    class CBooModel* m_parent = nullptr;
    CBooSurface* m_next = nullptr;

    zeus::CAABox GetBounds() const
    {
        if (!m_data.aabbSz)
            return zeus::CAABox(m_data.centroid, m_data.centroid);
        else
            return zeus::CAABox(m_data.aabb[0], m_data.aabb[1]);
    }
};

class CBooModel
{
    friend class CModel;
    friend class CBooRenderer;
    friend class CMetroidModelInstance;
    friend class CSkinnedModel;
public:
    using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;
    using UVAnimation = DataSpec::DNAMP1::MaterialSet::Material::UVAnimation;
    struct SShader
    {
        std::vector<TCachedToken<CTexture>> x0_textures;
        std::vector<std::vector<boo::IShaderPipeline*>> m_shaders;
        MaterialSet m_matSet;
        int m_matSetIdx;
        SShader(int idx) : m_matSetIdx(idx) {}
        void UnlockTextures();
    };

private:
    TLockedToken<CModel> m_model;
    std::vector<CBooSurface>* x0_surfaces;
    const MaterialSet* x4_matSet;
    int m_matSetIdx = -1;
    const std::vector<std::vector<boo::IShaderPipeline*>>* m_pipelines;
    boo::IVertexFormat* m_vtxFmt;
    boo::IGraphicsBufferS* x8_vbo;
    boo::IGraphicsBufferS* xc_ibo;
    size_t m_weightVecCount;
    size_t m_skinBankCount;
    std::vector<TCachedToken<CTexture>> x1c_textures;
    zeus::CAABox x20_aabb;
    CBooSurface* x38_firstUnsortedSurface = nullptr;
    CBooSurface* x3c_firstSortedSurface = nullptr;
    bool x40_24_texturesLoaded : 1;
    bool x40_25_modelVisible : 1;
    u8 x41_mask = 0;

    struct UVAnimationBuffer
    {
        static void ProcessAnimation(u8*& bufOut, const UVAnimation& anim);
        static void PadOutBuffer(u8*& bufStart, u8*& bufOut);
        static void Update(u8*& bufOut, const MaterialSet* matSet, const CModelFlags& flags);
    };

    CModelShaders::LightingUniform m_lightingData;

    /* urde addition: boo! */
    boo::GraphicsDataToken m_gfxToken;
    size_t m_uniformDataSize = 0;
    boo::IGraphicsBufferD* m_uniformBuffer = nullptr;
    std::vector<std::vector<boo::IShaderDataBinding*>> m_shaderDataBindings;

    void BuildGfxToken();
    void DrawAlphaSurfaces(const CModelFlags& flags) const;
    void DrawNormalSurfaces(const CModelFlags& flags) const;
    void DrawSurfaces(const CModelFlags& flags) const;
    void DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const;

    void VerifyCurrentShader(int shaderIdx);

public:
    CBooModel(TToken<CModel>& token, std::vector<CBooSurface>* surfaces, SShader& shader,
              boo::IVertexFormat* vtxFmt, boo::IGraphicsBufferS* vbo, boo::IGraphicsBufferS* ibo,
              size_t weightVecCount, size_t skinBankCount, const zeus::CAABox& aabb);

    static void MakeTexuresFromMats(const MaterialSet& matSet,
                                    std::vector<TCachedToken<CTexture>>& toksOut,
                                    IObjectStore& store);

    bool IsOpaque() const {return x3c_firstSortedSurface == nullptr;}
    void ActivateLights(const std::vector<CLight>& lights);
    void RemapMaterialData(SShader& shader);
    bool TryLockTextures() const;
    void UnlockTextures() const;
    void Touch(int shaderIdx) const;
    void UpdateUniformData(const CModelFlags& flags,
                           const CSkinRules* cskr,
                           const CPoseAsTransforms* pose) const;
    void DrawAlpha(const CModelFlags& flags,
                   const CSkinRules* cskr,
                   const CPoseAsTransforms* pose) const;
    void DrawNormal(const CModelFlags& flags,
                    const CSkinRules* cskr,
                    const CPoseAsTransforms* pose) const;
    void Draw(const CModelFlags& flags,
              const CSkinRules* cskr,
              const CPoseAsTransforms* pose) const;

    const MaterialSet::Material& GetMaterialByIndex(int idx) const
    {
        return x4_matSet->materials.at(idx);
    }

    static bool g_DrawingOccluders;
    static void SetDrawingOccluders(bool occ) {g_DrawingOccluders = occ;}
};

class CModel
{
    friend class CBooModel;
    //std::unique_ptr<u8[]> x0_data;
    //u32 x4_dataLen;
    TToken<CModel> m_selfToken; /* DO NOT LOCK! */
    zeus::CAABox m_aabb;
    std::vector<CBooSurface> x8_surfaces;
    std::vector<CBooModel::SShader> x18_matSets;
    std::unique_ptr<CBooModel> x28_modelInst;
    CModel* x30_next = nullptr;
    CModel* x34_prev = nullptr;

    /* urde addition: boo! */
    boo::GraphicsDataToken m_gfxToken;
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferS* m_ibo;
    boo::IVertexFormat* m_vtxFmt;

public:
    using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;

    CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store, CObjectReference* selfRef);
    void DrawSortedParts(const CModelFlags& flags) const;
    void DrawUnsortedParts(const CModelFlags& flags) const;
    void Draw(const CModelFlags& flags) const;
    bool IsLoaded(int shaderIdx) const;

    const zeus::CAABox& GetAABB() const {return m_aabb;}
    CBooModel& GetInstance() {return *x28_modelInst;}
    const CBooModel& GetInstance() const {return *x28_modelInst;}
    std::unique_ptr<CBooModel> MakeNewInstance(int shaderIdx);
};

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms,
                               CObjectReference* selfRef);

}

#endif // __URDE_CMODEL_HPP__
