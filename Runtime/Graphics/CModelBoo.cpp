#include "Graphics/CModel.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/HMDLMeta.hpp"

namespace urde
{
static logvisor::Module Log("urde::CModelBoo");
bool CBooModel::g_DrawingOccluders = false;

CBooModel::CBooModel(std::vector<CSurfaceView>* surfaces, std::vector<TLockedToken<CTexture>>* textures,
                     const u8* matSet, boo::IGraphicsBufferS* vbo, boo::IGraphicsBufferS* ibo, const zeus::CAABox& aabb,
                     u8 shortNormals, bool unk)
: x0_surfaces(surfaces), x4_matSet(matSet), x8_vbo(vbo), xc_ibo(ibo), x1c_textures(textures),
  x20_aabb(aabb), x40_24_(unk), x40_25_(0), x41_shortNormals(shortNormals)
{
    for (CSurfaceView& surf : *x0_surfaces)
        surf.m_parent = this;

    for (auto it=x0_surfaces->rbegin() ; it != x0_surfaces->rend() ; ++it)
    {
        u32 matId = hecl::SBig(*reinterpret_cast<const u32*>(it->m_data + 0xc));
        const u8* matData = GetMaterialByIndex(matId);
        u32 matFlags = hecl::SBig(*reinterpret_cast<const u32*>(matData));
        if (matFlags & 0x10)
        {
            it->m_next = x3c_firstSortedSurface;
            x3c_firstSortedSurface = &*it;
        }
        else
        {
            it->m_next = x38_firstUnsortedSurface;
            x38_firstUnsortedSurface = &*it;
        }
    }
}

void CBooModel::MakeTexuresFromMats(const u8* dataIn,
                                    std::vector<TLockedToken<CTexture>>& toksOut,
                                    IObjectStore& store)
{
    u32 texCount = hecl::SBig(*reinterpret_cast<const u32*>(dataIn));
    dataIn += 4;
    toksOut.reserve(texCount);
    for (u32 i=0 ; i<texCount ; ++i)
    {
        u32 id = hecl::SBig(*reinterpret_cast<const u32*>(dataIn));
        dataIn += 4;
        toksOut.emplace_back(store.GetObj({SBIG('TXTR'), id}));
    }
}

void CBooModel::TryLockTextures() const
{
}

void CBooModel::UnlockTextures() const
{
}

void CBooModel::DrawAlphaSurfaces(const CModelFlags& flags) const
{
}

void CBooModel::DrawNormalSurfaces(const CModelFlags& flags) const
{
}

void CBooModel::DrawSurfaces(const CModelFlags& flags) const
{
}

void CBooModel::DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const
{
}

void CBooModel::DrawAlpha(const CModelFlags& flags) const
{
}

void CBooModel::DrawNormal(const CModelFlags& flags) const
{
}

void CBooModel::Draw(const CModelFlags& flags) const
{
}

const u8* CBooModel::GetMaterialByIndex(int idx) const
{
    const u32* matOffs = reinterpret_cast<const u32*>(x4_matSet + (x1c_textures->size() + 1) * 4);
    u32 matCount = hecl::SBig(*matOffs);
    ++matOffs;

    const u8* materialBase = reinterpret_cast<const u8*>(matOffs + matCount);
    if (idx == 0)
        return materialBase;

    u32 offset = hecl::SBig(matOffs[idx-1]);
    return materialBase + offset;
}

static const u8* MemoryFromPartData(const u8*& dataCur, const s32*& secSizeCur)
{
    const u8* ret;
    if (*secSizeCur)
        ret = dataCur;
    else
        ret = nullptr;

    dataCur += hecl::SBig(*secSizeCur);
    ++secSizeCur;
    return ret;
}

CModel::CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store)
: x0_data(std::move(in)), x4_dataLen(dataLen)
{
    u32 version = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x4));
    u32 flags = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x8));
    if (version != 16)
        Log.report(logvisor::Fatal, "invalid CMDL for loading with boo");

    u32 secCount = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x24));
    u32 matSetCount = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x28));
    x18_matSets.reserve(matSetCount);
    const u8* dataCur = x0_data.get() + ROUND_UP_32(0x2c + secCount * 4);
    const s32* secSizeCur = reinterpret_cast<const s32*>(x0_data.get() + 0x2c);
    for (u32 i=0 ; i<matSetCount ; ++i)
    {
        const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
        x18_matSets.emplace_back();
        SShader& shader = x18_matSets.back();
        shader.x10_data = sec;
        CBooModel::MakeTexuresFromMats(sec, shader.x0_textures, *store);
    }

    hecl::HMDLMeta hmdlMeta;
    {
        const u8* hmdlMetadata = MemoryFromPartData(dataCur, secSizeCur);
        athena::io::MemoryReader r(hmdlMetadata, *secSizeCur);
        hmdlMeta.read(r);
    }

    const u8* vboData = MemoryFromPartData(dataCur, secSizeCur);
    const u8* iboData = MemoryFromPartData(dataCur, secSizeCur);
    const u8* surfInfo = MemoryFromPartData(dataCur, secSizeCur);

    m_gfxToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, vboData, hmdlMeta.vertStride, hmdlMeta.vertCount);
        m_ibo = ctx.newStaticBuffer(boo::BufferUse::Index, iboData, 4, hmdlMeta.indexCount);
        return true;
    });

    u32 surfCount = hecl::SBig(*reinterpret_cast<const u32*>(surfInfo));
    x8_surfaces.reserve(surfCount);
    for (u32 i=0 ; i<surfCount ; ++i)
    {
        const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
        x8_surfaces.emplace_back();
        CBooModel::CSurfaceView& surf = x8_surfaces.back();
        surf.m_data = sec;
    }

    const float* aabbPtr = reinterpret_cast<const float*>(x0_data.get() + 0x18);
    zeus::CAABox aabb(hecl::SBig(aabbPtr[0]), hecl::SBig(aabbPtr[1]), hecl::SBig(aabbPtr[2]),
                      hecl::SBig(aabbPtr[3]), hecl::SBig(aabbPtr[4]), hecl::SBig(aabbPtr[5]));
    x28_modelInst = std::make_unique<CBooModel>(&x8_surfaces, &x18_matSets[0].x0_textures,
                                                x18_matSets[0].x10_data, m_vbo, m_ibo,
                                                aabb, flags & 0x2, true);
}

void CModel::Draw(const CModelFlags& flags) const
{
}

void CModel::Touch(int) const
{
}

bool CModel::IsLoaded(int) const
{
    return false;
}

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms)
{
    IObjectStore* store = static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam();
    return TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, store));
}

}
