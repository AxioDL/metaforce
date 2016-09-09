#include "BlenderConnection.hpp"
#include <cmath>
#include <float.h>

namespace hecl
{

atVec3f BlenderConnection::DataStream::MtxVecMul4RM(const Matrix4f& mtx, const Vector3f& vec)
{
    atVec3f res;
    res.vec[0] = mtx[0].vec[0] * vec.val.vec[0] + mtx[0].vec[1] * vec.val.vec[1] + mtx[0].vec[2] * vec.val.vec[2] + mtx[0].vec[3];
    res.vec[1] = mtx[1].vec[0] * vec.val.vec[0] + mtx[1].vec[1] * vec.val.vec[1] + mtx[1].vec[2] * vec.val.vec[2] + mtx[1].vec[3];
    res.vec[2] = mtx[2].vec[0] * vec.val.vec[0] + mtx[2].vec[1] * vec.val.vec[1] + mtx[2].vec[2] * vec.val.vec[2] + mtx[2].vec[3];
    return res;
}

atVec3f BlenderConnection::DataStream::MtxVecMul3RM(const Matrix4f& mtx, const Vector3f& vec)
{
    atVec3f res;
    res.vec[0] = mtx[0].vec[0] * vec.val.vec[0] + mtx[0].vec[1] * vec.val.vec[1] + mtx[0].vec[2] * vec.val.vec[2];
    res.vec[1] = mtx[1].vec[0] * vec.val.vec[0] + mtx[1].vec[1] * vec.val.vec[1] + mtx[1].vec[2] * vec.val.vec[2];
    res.vec[2] = mtx[2].vec[0] * vec.val.vec[0] + mtx[2].vec[1] * vec.val.vec[1] + mtx[2].vec[2] * vec.val.vec[2];
    return res;
}

HMDLBuffers BlenderConnection::DataStream::Mesh::getHMDLBuffers(bool absoluteCoords) const
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
    metaOut.bankCount = skinBanks.banks.size();

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
    HMDLBuffers ret(std::move(metaOut), vboSz, iboData, std::move(outSurfaces), skinBanks);
    athena::io::MemoryWriter vboW(ret.m_vboData.get(), vboSz);
    for (const std::pair<const Surface*, const Surface::Vert*>& sv : vertPool)
    {
        const Surface& s = *sv.first;
        const Surface::Vert& v = *sv.second;

        if (absoluteCoords)
        {
            atVec3f preXfPos = MtxVecMul4RM(sceneXf, pos[v.iPos]);
            vboW.writeVec3fLittle(preXfPos);

            atVec3f preXfNorm = MtxVecMul3RM(sceneXf, norm[v.iNorm]);
            float mag =
                preXfNorm.vec[0] * preXfNorm.vec[0] +
                preXfNorm.vec[1] * preXfNorm.vec[1] +
                preXfNorm.vec[2] * preXfNorm.vec[2];
            if (mag > FLT_EPSILON)
                mag = 1.f / std::sqrt(mag);
            preXfNorm.vec[0] *= mag;
            preXfNorm.vec[1] *= mag;
            preXfNorm.vec[2] *= mag;
            vboW.writeVec3fLittle(preXfNorm);
        }
        else
        {
            vboW.writeVec3fLittle(pos[v.iPos]);
            vboW.writeVec3fLittle(norm[v.iNorm]);
        }

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
