#include "Graphics/CModel.hpp"

namespace urde
{
static logvisor::Module Log("urde::CModelBoo");

CBooModel::CBooModel(std::vector<CSurface>* surfaces, std::vector<TLockedToken<CTexture>>* textures,
                     const u8* matSet, const void* vbo, const void* ibo, const zeus::CAABox& aabb,
                     u8 shortNormals, bool unk)
: x0_surfaces(surfaces), x4_matSet(matSet), x8_vbo(vbo), xc_ibo(ibo), x1c_textures(textures),
  x20_aabb(aabb), x40_24_(unk), x40_25_(0), x41_shortNormals(shortNormals)
{
    for (CSurface& surf : *x0_surfaces)
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

    const u8* vboData = MemoryFromPartData(dataCur, secSizeCur);
    const u8* iboData = MemoryFromPartData(dataCur, secSizeCur);
    const u8* surfInfo = MemoryFromPartData(dataCur, secSizeCur);

    u32 surfCount = hecl::SBig(*reinterpret_cast<const u32*>(surfInfo));
    x8_surfaces.reserve(surfCount);
    for (u32 i=0 ; i<surfCount ; ++i)
    {
        const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
        x8_surfaces.emplace_back();
        CBooModel::CSurface& surf = x8_surfaces.back();
        surf.m_data = sec;
    }

    const float* aabbPtr = reinterpret_cast<const float*>(x0_data.get() + 0x18);
    zeus::CAABox aabb(hecl::SBig(aabbPtr[0]), hecl::SBig(aabbPtr[1]), hecl::SBig(aabbPtr[2]),
                      hecl::SBig(aabbPtr[3]), hecl::SBig(aabbPtr[4]), hecl::SBig(aabbPtr[5]));
    x28_modelInst = std::make_unique<CBooModel>(&x8_surfaces, &x18_matSets[0].x0_textures,
                                                x18_matSets[0].x10_data, vboData, iboData,
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
