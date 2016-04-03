#ifndef __PSHAG_CMODEL_HPP__
#define __PSHAG_CMODEL_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "CFactoryMgr.hpp"
#include "CToken.hpp"
#include "zeus/CAABox.hpp"
#include "DNACommon/CMDL.hpp"
#include "DNAMP1/CMDLMaterials.hpp"

#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace urde
{
class IObjectStore;
class CTexture;

struct CModelFlags
{
    u8 m_blendMode = 0; /* Blend state 3/5 enable additive */
    u8 m_matSetIdx = 0;
    u16 m_flags = 0; /* Flags */
    zeus::CColor color; /* Set into kcolor slot specified by material */

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
    DataSpec::DNACMDL::SurfaceHeader_1 m_data;
    class CBooModel* m_parent = nullptr;
    CBooSurface* m_next = nullptr;
};

struct SUnskinnedXf
{
    zeus::CMatrix4f mv;
    zeus::CMatrix4f mvinv;
    zeus::CMatrix4f proj;
};

class CBooModel
{
    friend class CModel;
public:
    using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;
    using UVAnimation = DataSpec::DNAMP1::MaterialSet::Material::UVAnimation;
    struct SShader
    {
        std::vector<TCachedToken<CTexture>> x0_textures;
        std::vector<boo::IShaderPipeline*> m_shaders;
        MaterialSet m_matSet;
        void UnlockTextures();
    };

private:
    std::vector<CBooSurface>* x0_surfaces;
    const MaterialSet* x4_matSet;
    const std::vector<boo::IShaderPipeline*>* m_pipelines;
    boo::IVertexFormat* m_vtxFmt;
    boo::IGraphicsBufferS* x8_vbo;
    boo::IGraphicsBufferS* xc_ibo;
    std::vector<TCachedToken<CTexture>>* x1c_textures;
    zeus::CAABox x20_aabb;
    CBooSurface* x38_firstUnsortedSurface = nullptr;
    CBooSurface* x3c_firstSortedSurface = nullptr;
    bool x40_24_texturesLoaded : 1;
    bool x40_25_ : 1;
    u8 x41_shortNormals;

    struct UVAnimationBuffer
    {
        std::vector<zeus::CMatrix4f> m_buffer;
        std::vector<std::pair<size_t,size_t>> m_ranges;
        void ProcessAnimation(const UVAnimation& anim);
        void PadOutBuffer();
        void Update(const MaterialSet* matSet);
        operator bool() const {return m_buffer.size() != 0;}
    } m_uvAnimBuffer;

    /* urde addition: boo! */
    boo::GraphicsDataToken m_gfxToken;
    boo::IGraphicsBufferD* m_unskinnedXfBuffer;
    boo::IGraphicsBufferD* m_uvMtxBuffer;
    std::vector<boo::IShaderDataBinding*> m_shaderDataBindings;

    void BuildGfxToken();
    void UpdateUniformData() const;
    void DrawAlphaSurfaces(const CModelFlags& flags) const;
    void DrawNormalSurfaces(const CModelFlags& flags) const;
    void DrawSurfaces(const CModelFlags& flags) const;
    void DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const;

public:
    CBooModel(std::vector<CBooSurface>* surfaces, SShader& shader,
              boo::IVertexFormat* vtxFmt, boo::IGraphicsBufferS* vbo, boo::IGraphicsBufferS* ibo,
              const zeus::CAABox& aabb,
              u8 shortNormals, bool texturesLoaded);

    static void MakeTexuresFromMats(const MaterialSet& matSet,
                                    std::vector<TCachedToken<CTexture>>& toksOut,
                                    IObjectStore& store);

    void RemapMaterialData(SShader& shader);
    bool TryLockTextures() const;
    void UnlockTextures() const;
    void DrawAlpha(const CModelFlags& flags) const;
    void DrawNormal(const CModelFlags& flags) const;
    void Draw(const CModelFlags& flags) const;

    const MaterialSet::Material& GetMaterialByIndex(int idx) const
    {
        return x4_matSet->materials.at(idx);
    }

    static bool g_DrawingOccluders;
    static void SetDrawingOccluders(bool occ) {g_DrawingOccluders = occ;}
};

class CModel
{
    std::unique_ptr<u8[]> x0_data;
    u32 x4_dataLen;
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

    void VerifyCurrentShader(int shaderIdx) const;

public:
    using MaterialSet = DataSpec::DNAMP1::HMDLMaterialSet;

    CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store);
    void DrawSortedParts(const CModelFlags& flags) const;
    void DrawUnsortedParts(const CModelFlags& flags) const;
    void Draw(const CModelFlags& flags) const;
    void Touch(int shaderIdx) const;
    bool IsLoaded(int shaderIdx) const;
};

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms);

}

#endif // __PSHAG_CMODEL_HPP__
