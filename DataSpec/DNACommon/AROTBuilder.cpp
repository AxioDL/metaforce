#include "AROTBuilder.hpp"

namespace DataSpec
{

static const uint32_t AROTChildCounts[] = { 0, 2, 2, 4, 2, 4, 4, 8 };

size_t AROTBuilder::BitmapPool::addIndices(const std::set<int>& indices)
{
    for (size_t i=0 ; i<m_pool.size() ; ++i)
        if (m_pool[i] == indices)
            return i;
    m_pool.push_back(indices);
    return m_pool.size() - 1;
}

bool AROTBuilder::Node::addChild(const zeus::CAABox& curAabb, const zeus::CAABox& childAabb, int idx)
{
    if (childAabb.intersects(curAabb))
    {
        childIndices.insert(idx);
        if (!curAabb.inside(childAabb))
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
                    inZ[0] = childNodes[i*4 + j*2].addChild(Z[0], childAabb, idx);
                    inZ[1] = childNodes[i*4 + j*2 + 1].addChild(Z[1], childAabb, idx);
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
            nodeSz += childCount * 2;
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

void AROTBuilder::build(std::vector<std::vector<uint8_t>>& secs, const zeus::CAABox& fullAabb,
                        const std::vector<zeus::CAABox>& meshAabbs, const std::vector<DNACMDL::Mesh>& meshes)
{
    for (int i=0 ; i<meshAabbs.size() ; ++i)
    {
        const zeus::CAABox& aabb = meshAabbs[i];
        rootNode.addChild(fullAabb, aabb, i);
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

}
