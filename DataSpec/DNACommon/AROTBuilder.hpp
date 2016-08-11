#ifndef _DNACOMMON_AROTBUILDER_HPP_
#define _DNACOMMON_AROTBUILDER_HPP_

#include "DNACommon.hpp"
#include "DeafBabe.hpp"
#include "zeus/CAABox.hpp"
#include "CMDL.hpp"
#include <set>

namespace DataSpec
{

struct AROTBuilder
{
    struct BitmapPool
    {
        std::vector<std::set<int>> m_pool;
        size_t addIndices(const std::set<int>& indices);
    } bmpPool;

    struct Node
    {
        std::vector<Node> childNodes;
        std::set<int> childIndices;
        size_t poolIdx = 0;
        uint8_t flags = 0;

        size_t nodeOff = 0;
        size_t nodeSz = 4;

        bool addChild(int level, const zeus::CAABox& curAabb, const zeus::CAABox& childAabb, int idx);
        void nodeCount(size_t& sz, size_t& idxRefs, BitmapPool& bmpPool, size_t& curOff);
        void writeIndirectionTable(athena::io::MemoryWriter& w);
        void writeNodes(athena::io::MemoryWriter& w, int nodeIdx);
        void advanceIndex(int& nodeIdx);

        void colSize(size_t& totalSz);
        void writeColNodes(uint8_t*& ptr, const zeus::CAABox& curAABB);
        uint16_t getColRef(uint32_t& offset);
    } rootNode;

    void build(std::vector<std::vector<uint8_t>>& secs, const zeus::CAABox& fullAabb,
               const std::vector<zeus::CAABox>& meshAabbs, const std::vector<DNACMDL::Mesh>& meshes);
    using ColMesh = hecl::BlenderConnection::DataStream::ColMesh;
    std::pair<std::unique_ptr<uint8_t[]>, uint32_t> buildCol(const ColMesh& mesh, BspNodeType& rootOut);
};

}

#endif // _DNACOMMON_AROTBUILDER_HPP_
