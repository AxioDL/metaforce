#include "AROTBuilder.hpp"

namespace DataSpec
{
logvisor::Module Log("AROTBuilder");

#define AROT_MAX_LEVEL 7

static const uint32_t AROTChildCounts[] = { 0, 2, 2, 4, 2, 4, 4, 8 };

size_t AROTBuilder::BitmapPool::addIndices(const std::set<int>& indices)
{
    for (size_t i=0 ; i<m_pool.size() ; ++i)
        if (m_pool[i] == indices)
            return i;
    m_pool.push_back(indices);
    return m_pool.size() - 1;
}

bool AROTBuilder::Node::addChild(int level, const zeus::CAABox& curAabb, const zeus::CAABox& childAabb, int idx)
{
    if (childAabb.intersects(curAabb))
    {
        childIndices.insert(idx);
        if (!curAabb.inside(childAabb) && level < AROT_MAX_LEVEL)
        {
            childNodes.resize(8);
            zeus::CAABox X[2];
            curAabb.splitX(X[0], X[1]);
            bool inX[2] = {};
            for (int i=0 ; i<2 ; ++i)
            {
                zeus::CAABox Y[2];
                X[i].splitY(Y[0], Y[1]);
                bool inY[2] = {};
                for (int j=0 ; j<2 ; ++j)
                {
                    zeus::CAABox Z[2];
                    Y[j].splitZ(Z[0], Z[1]);
                    bool inZ[2] = {};
                    inZ[0] = childNodes[i*4 + j*2].addChild(level + 1, Z[0], childAabb, idx);
                    inZ[1] = childNodes[i*4 + j*2 + 1].addChild(level + 1, Z[1], childAabb, idx);
                    if (inZ[0] ^ inZ[1])
                        flags |= 0x4;
                    if (inZ[0] | inZ[1])
                        inY[j] = true;
                }
                if (inY[0] ^ inY[1])
                    flags |= 0x2;
                if (inY[0] | inY[1])
                    inX[i] = true;
            }
            if (inX[0] ^ inX[1])
                flags |= 0x1;

            if (!flags)
                childNodes.clear();
        }
        return true;
    }
    return false;
}

void AROTBuilder::Node::nodeCount(size_t& sz, size_t& idxRefs, BitmapPool& bmpPool, size_t& curOff)
{
    if (childIndices.size())
    {
        sz += 1;
        poolIdx = bmpPool.addIndices(childIndices);
        if (poolIdx > 65535)
            Log.report(logvisor::Fatal, "AROT bitmap exceeds 16-bit node addressing; area too complex");

        if (childNodes.size())
        {
            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        childNodes[i*4 + j*2 + k].nodeCount(sz, idxRefs, bmpPool, curOff);
                    }
                }
            }
            uint32_t childCount = AROTChildCounts[flags];
            nodeOff = curOff;
            nodeSz = childCount * 2 + 4;
            curOff += nodeSz;
            idxRefs += childCount;
        }
    }
}

void AROTBuilder::Node::writeIndirectionTable(athena::io::MemoryWriter& w)
{
    if (childIndices.size())
    {
        w.writeUint32Big(nodeOff);
        if (childNodes.size())
        {
            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        childNodes[i*4 + j*2 + k].writeIndirectionTable(w);
                    }
                }
            }
        }
    }
}

void AROTBuilder::Node::writeNodes(athena::io::MemoryWriter& w, int nodeIdx)
{
    if (childIndices.size())
    {
        w.writeUint16Big(poolIdx);
        w.writeUint16Big(flags);
        if (childNodes.size())
        {
            int curIdx = nodeIdx + 1;
            if (curIdx > 65535)
                Log.report(logvisor::Fatal, "AROT node exceeds 16-bit node addressing; area too complex");

            int childIndices[8];

            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        int ch = i*4 + j*2 + k;
                        w.writeUint16Big(curIdx);
                        childIndices[ch] = curIdx;
                        childNodes[ch].advanceIndex(curIdx);
                    }
                }
            }

            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        int ch = i*4 + j*2 + k;
                        childNodes[ch].writeNodes(w, childIndices[ch]);
                    }
                }
            }
        }
    }
}

void AROTBuilder::Node::advanceIndex(int& nodeIdx)
{
    if (childIndices.size())
    {
        ++nodeIdx;
        if (childNodes.size())
        {
            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        childNodes[i*4 + j*2 + k].advanceIndex(nodeIdx);
                    }
                }
            }
        }
    }
}

void AROTBuilder::Node::colSize(size_t& totalSz)
{
    if (childIndices.size())
    {
        nodeOff = totalSz;
        if (childNodes.empty())
        {
            totalSz += 26 + childIndices.size() * 2;
        }
        else
        {
            totalSz += 36;
            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        childNodes[i*4 + j*2 + k].colSize(totalSz);
                    }
                }
            }
        }
    }
}

void AROTBuilder::Node::writeColNodes(uint8_t*& ptr, const zeus::CAABox& curAABB)
{
    if (childIndices.size())
    {
        if (childNodes.empty())
        {
            zeus::CAABox swapAABB = curAABB;
            swapAABB.min[0] = hecl::SBig(swapAABB.min[0]);
            swapAABB.min[1] = hecl::SBig(swapAABB.min[1]);
            swapAABB.min[2] = hecl::SBig(swapAABB.min[2]);
            swapAABB.max[0] = hecl::SBig(swapAABB.max[0]);
            swapAABB.max[1] = hecl::SBig(swapAABB.max[1]);
            swapAABB.max[2] = hecl::SBig(swapAABB.max[2]);
            *reinterpret_cast<zeus::CVector3f*>(ptr) = swapAABB.min;
            *reinterpret_cast<zeus::CVector3f*>(ptr + 12) = swapAABB.max;
            athena::io::MemoryWriter w(ptr + 24, INT32_MAX);
            w.writeUint16Big(childIndices.size());
            for (int idx : childIndices)
                w.writeUint16Big(idx);
            ptr += 26 + childIndices.size() * 2;
        }
        else
        {
            uint16_t* pflags = reinterpret_cast<uint16_t*>(ptr);
            uint32_t* offsets = reinterpret_cast<uint32_t*>(ptr + 4);
            *pflags = 0;
            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        int idx = i*4 + j*2 + k;
                        uint32_t thisOffset;
                        uint16_t thisFlags = childNodes[idx].getColRef(thisOffset);
                        if (thisFlags)
                        {
                            *pflags |= thisFlags << (idx * 2);
                            offsets[idx] = hecl::SBig(uint32_t(thisOffset - nodeOff - 36));
                        }
                    }
                }
            }
            *pflags = hecl::SBig(*pflags);
            ptr += 36;

            zeus::CAABox X[2];
            if (flags & 0x1)
                curAABB.splitX(X[0], X[1]);
            else
            {
                X[0] = curAABB;
                X[1] = curAABB;
            }

            for (int i=0 ; i < 1 + ((flags & 0x1) != 0) ; ++i)
            {
                zeus::CAABox Y[2];
                if (flags & 0x2)
                    X[i].splitY(Y[0], Y[1]);
                else
                {
                    Y[0] = X[i];
                    Y[1] = X[i];
                }

                for (int j=0 ; j < 1 + ((flags & 0x2) != 0) ; ++j)
                {
                    zeus::CAABox Z[2];
                    if (flags & 0x4)
                        Y[j].splitZ(Z[0], Z[1]);
                    else
                    {
                        Z[0] = Y[j];
                        Z[1] = Y[j];
                    }

                    for (int k=0 ; k < 1 + ((flags & 0x4) != 0) ; ++k)
                    {
                        int idx = i*4 + j*2 + k;
                        childNodes[idx].writeColNodes(ptr, Z[k]);
                    }
                }
            }
        }
    }
}

uint16_t AROTBuilder::Node::getColRef(uint32_t& offset)
{
    if (childIndices.size())
    {
        offset = nodeOff;
        if (childNodes.empty())
            return 2;
        else
            return 1;
    }
    return 0;
}

void AROTBuilder::build(std::vector<std::vector<uint8_t>>& secs, const zeus::CAABox& fullAabb,
                        const std::vector<zeus::CAABox>& meshAabbs, const std::vector<DNACMDL::Mesh>& meshes)
{
    for (int i=0 ; i<meshAabbs.size() ; ++i)
    {
        const zeus::CAABox& aabb = meshAabbs[i];
        rootNode.addChild(0, fullAabb, aabb, i);
    }

    size_t totalNodeCount = 0;
    size_t idxRefCount = 0;
    size_t curOff = 0;
    rootNode.nodeCount(totalNodeCount, idxRefCount, bmpPool, curOff);
    size_t bmpWordCount = ROUND_UP_32(meshes.size()) / 32;
    size_t arotSz = 64 + bmpWordCount * bmpPool.m_pool.size() * 4 + totalNodeCount * 8 + idxRefCount * 2;

    secs.emplace_back(arotSz, 0);
    athena::io::MemoryWriter w(secs.back().data(), secs.back().size());
    w.writeUint32Big('AROT');
    w.writeUint32Big(1);
    w.writeUint32Big(bmpPool.m_pool.size());
    w.writeUint32Big(meshes.size());
    w.writeUint32Big(totalNodeCount);
    w.writeVec3fBig(fullAabb.min);
    w.writeVec3fBig(fullAabb.max);
    w.seekAlign32();

    std::vector<uint32_t> bmpWords;
    bmpWords.reserve(bmpWordCount);
    for (const std::set<int>& bmp : bmpPool.m_pool)
    {
        bmpWords.clear();
        bmpWords.resize(bmpWordCount);

        auto bmpIt = bmp.cbegin();
        if (bmpIt != bmp.cend())
        {
            int curIdx = 0;
            for (int w=0 ; w<bmpWordCount ; ++w)
            {
                for (int b=0 ; b<32 ; ++b)
                {
                    if (*bmpIt == curIdx)
                    {
                        bmpWords[w] |= 1 << b;
                        ++bmpIt;
                        if (bmpIt == bmp.cend())
                            break;
                    }
                    ++curIdx;
                }
                if (bmpIt == bmp.cend())
                    break;
            }
        }

        for (uint32_t word : bmpWords)
            w.writeUint32Big(word);
    }

    rootNode.writeIndirectionTable(w);
    rootNode.writeNodes(w, 0);
}

std::pair<std::unique_ptr<uint8_t[]>, uint32_t> AROTBuilder::buildCol(const ColMesh& mesh, BspNodeType& rootOut)
{
    zeus::CAABox fullAabb(zeus::CVector3f(mesh.aabbMin), zeus::CVector3f(mesh.aabbMax));

    int t = 0;
    for (const ColMesh::Triangle& tri : mesh.trianges)
    {
        zeus::CAABox aabb;
        for (int e=0 ; e<3 ; ++e)
        {
            const ColMesh::Edge& edge = mesh.edges[tri.edges[e]];
            for (int v=0 ; v<2 ; ++v)
            {
                const auto& vert = mesh.verts[edge.verts[v]];
                aabb.accumulateBounds(zeus::CVector3f(vert));
            }
        }
        rootNode.addChild(0, fullAabb, aabb, t);
        ++t;
    }

    size_t totalSize = 0;
    rootNode.colSize(totalSize);
    std::unique_ptr<uint8_t[]> ret(new uint8_t[totalSize]);
    uint32_t dummy;
    rootOut = BspNodeType(rootNode.getColRef(dummy));
    uint8_t* ptr = ret.get();
    rootNode.writeColNodes(ptr, fullAabb);

    return {std::move(ret), totalSize};
}

}
