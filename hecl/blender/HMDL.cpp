#include "BlenderConnection.hpp"

namespace HECL
{

HMDLBuffers BlenderConnection::DataStream::Mesh::getHMDLBuffers() const
{
    /* If skinned, compute max weight vec count */
    size_t weightCount = 0;
    for (const SkinBanks::Bank& bank : skinBanks.banks)
        weightCount = std::max(weightCount, bank.m_boneIdxs.size());
    size_t weightVecCount = weightCount / 4;
    if (weightCount % 4)
        ++weightVecCount;

    /* Prepare HMDL meta */
    HMDLMeta metaOut;
    metaOut.topology = topology;
    metaOut.vertStride = (3 + 3 + colorLayerCount + uvLayerCount * 2 + weightVecCount * 4) * 4;
    metaOut.colorCount = colorLayerCount;
    metaOut.uvCount = uvLayerCount;
    metaOut.weightCount = weightVecCount;

    /* Total all verts from all surfaces (for ibo length) */
    size_t boundVerts = 0;
    for (const Surface& surf : surfaces)
        boundVerts += surf.verts.size();

    /* Maintain unique vert pool for VBO */
    std::vector<std::pair<const Surface*, const Surface::Vert*>> vertPool;
    vertPool.reserve(boundVerts);

    /* Target surfaces representation */
    std::vector<HMDLBuffers::Surface> outSurfaces;
    outSurfaces.reserve(surfaces.size());

    /* Index buffer */
    std::vector<atUint32> iboData;
    iboData.reserve(boundVerts);

    for (const Surface& surf : surfaces)
    {
        size_t iboStart = iboData.size();
        for (const Surface::Vert& v : surf.verts)
        {
            size_t ti = 0;
            bool found = false;
            for (const std::pair<const Surface*, const Surface::Vert*>& tv : vertPool)
            {
                if (v == *tv.second)
                {
                    iboData.push_back(ti);
                    found = true;
                    break;
                }
                ++ti;
            }
            if (!found)
            {
                iboData.push_back(vertPool.size());
                vertPool.emplace_back(&surf, &v);
            }
        }
        outSurfaces.emplace_back(surf, iboStart, iboData.size() - iboStart);
    }

    metaOut.vertCount = vertPool.size();
    metaOut.indexCount = iboData.size();

    size_t vboSz = metaOut.vertCount * metaOut.vertStride;
    HMDLBuffers ret(metaOut, vboSz, iboData, std::move(outSurfaces), skinBanks);
    Athena::io::MemoryWriter vboW(ret.m_vboData.get(), vboSz);
    for (const std::pair<const Surface*, const Surface::Vert*>& sv : vertPool)
    {
        const Surface& s = *sv.first;
        const Surface::Vert& v = *sv.second;

        vboW.writeVec3fLittle(pos[v.iPos]);
        vboW.writeVec3fLittle(norm[v.iNorm]);

        for (size_t i=0 ; i<colorLayerCount ; ++i)
        {
            const Vector3f& c = color[v.iColor[i]];
            vboW.writeUByte(std::max(0, std::min(255, int(c.val.vec[0] * 255))));
            vboW.writeUByte(std::max(0, std::min(255, int(c.val.vec[1] * 255))));
            vboW.writeUByte(std::max(0, std::min(255, int(c.val.vec[2] * 255))));
            vboW.writeUByte(255);
        }

        for (size_t i=0 ; i<uvLayerCount ; ++i)
            vboW.writeVec2fLittle(uv[v.iUv[i]]);

        if (weightVecCount)
        {
            const SkinBanks::Bank& bank = skinBanks.banks[s.skinBankIdx];
            const std::vector<SkinBind>& binds = skins[v.iSkin];
            auto it = bank.m_boneIdxs.cbegin();
            for (size_t i=0 ; i<weightVecCount ; ++i)
            {
                atVec4f vec = {};
                for (size_t j=0 ; j<4 ; ++j)
                {
                    if (it == bank.m_boneIdxs.cend())
                        break;
                    for (const SkinBind& bind : binds)
                        if (bind.boneIdx == *it)
                        {
                            vec.vec[j] = bind.weight;
                            break;
                        }
                    ++it;
                }
                vboW.writeVec4fLittle(vec);
            }
        }
    }

    return ret;
}

}
