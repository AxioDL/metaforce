#include "MetroidPrime/PathFinding/CPathFindArea.hpp"

#include "Metaforce/CResourceReader.hpp"
#include "Metaforce/Common.hpp"

#include <algorithm>
#include <array>
#include <vector>

namespace {
constexpr borealis::Log Log{"CPFArea"};

void CheckRange(uint first, uint count, size_t size) {
  REQUIRE(first <= size && count <= size - first, "PATH range {} + {} exceeds array of {} records",
          first, count, size);
}
} // namespace

void CPFArea::ReadData(std::span< const uchar > data) {
  CResourceReader in(data, Log.name);
  const uint version = in.Read< uint >();
  REQUIRE(version == 4, "Unsupported PATH version {}", version);

  mNodes.resize(in.ReadCount(24), CPFNode());
  for (auto& node : mNodes) {
    node.mPosition = in.ReadVector3f();
    node.mNormal = in.ReadVector3f();
  }
  mLinks.resize(in.ReadCount(16), CPFLink());
  for (auto& link : mLinks) {
    link.mNode = in.Read< int >();
    link.mRegion = in.Read< int >();
    link.x8_2dWidth = in.Read< float >();
    link.mOo2dWidth = in.Read< float >();
  }

  const uint regionCount = in.ReadCount(80);
  REQUIRE(regionCount <= 512, "PATH has {} regions; search bitsets hold 512", regionCount);
  mRegions.resize(regionCount, CPFRegion());
  mRegionData.resize(regionCount, CPFRegionData());
  std::array< bool, 512 > regionIndices{};
  int maxRegionNodes = 4;
  for (auto& region : mRegions) {
    const uint nodeCount = in.Read< uint >();
    const uint firstNode = in.Read< uint >();
    const uint linkCount = in.Read< uint >();
    const uint firstLink = in.Read< uint >();
    REQUIRE(nodeCount != 0, "PATH region has no polygon nodes");
    CheckRange(firstNode, nodeCount, mNodes.size());
    if (linkCount != 0) {
      CheckRange(firstLink, linkCount, mLinks.size());
    }
    region.mNumNodes = nodeCount;
    region.mStartNode = mNodes.data() + firstNode;
    region.mNumLinks = linkCount;
    region.mStartLink = linkCount ? mLinks.data() + firstLink : nullptr;
    region.mFlags = in.Read< uint >();
    region.mHeight = in.Read< float >();
    region.mNormal = in.ReadVector3f();
    const uint index = in.Read< uint >();
    REQUIRE(index < regionCount && !regionIndices[index], "Invalid PATH region index {}", index);
    regionIndices[index] = true;
    region.mRegionIdx = index;
    region.mCentroid = in.ReadVector3f();
    region.mBounds = in.ReadAABox();
    in.Read< uint >(); // The serialized scratch-data pointer is not persistent data.
    region.mData = &mRegionData[index];
    maxRegionNodes = std::max(maxRegionNodes, region.mNumNodes);
    for (uint i = 0; i < linkCount; ++i) {
      const auto& link = region.mStartLink[i];
      REQUIRE(link.GetNode() >= 0 && static_cast< uint >(link.GetNode()) < nodeCount &&
                  link.GetRegion() >= 0 && static_cast< uint >(link.GetRegion()) < regionCount,
              "Invalid PATH link in region {}", index);
    }
  }
  mPolyPoints.reserve(maxRegionNodes);

  const uint connectionWords = regionCount ? (regionCount * (regionCount - 1) / 2 + 31) / 32 : 0;
  mConnectionsGround.reserve(connectionWords);
  mConnectionsFlyers.reserve(connectionWords);
  for (uint i = 0; i < connectionWords; ++i) {
    mConnectionsGround.push_back(in.Read< uint >());
  }
  for (uint i = 0; i < connectionWords; ++i) {
    mConnectionsFlyers.push_back(in.Read< uint >());
  }
  // The file reserves two square bit matrices; the game uses triangular tables.
  in.Take(((regionCount * regionCount + 31) / 32 - connectionWords) * 2 * sizeof(uint));

  const uint lookupCount = in.ReadCount(4);
  mOctreeRegions.reserve(lookupCount);
  for (uint i = 0; i < lookupCount; ++i) {
    const uint index = in.Read< uint >();
    REQUIRE(index < regionCount, "Invalid PATH octree region index {}", index);
    mOctreeRegions.push_back(&mRegions[index]);
  }
  const uint octreeCount = in.ReadCount(80);
  REQUIRE(octreeCount != 0, "PATH has no octree root");
  mOctree.resize(octreeCount, CPFAreaOctree());
  for (auto& node : mOctree) {
    node.mIsLeaf = in.Read< uint >() != 0;
    node.mBounds = in.ReadAABox();
    node.mCenter = in.ReadVector3f();
    for (auto& child : node.mChildren) {
      const uint index = in.Read< uint >();
      if (!node.mIsLeaf) {
        // Traversal visits all eight children without testing for null.
        REQUIRE(index < octreeCount, "Invalid PATH octree child {}", index);
        child = &mOctree[index];
      } else {
        child = nullptr;
      }
    }
    const uint count = in.Read< uint >();
    const uint firstRegion = in.Read< uint >();
    if (node.mIsLeaf && count != 0) {
      CheckRange(firstRegion, count, lookupCount);
      node.mRegions.set_size(count);
      node.mRegions.set_data(mOctreeRegions.data() + firstRegion);
    }
  }

  std::vector< uchar > state(octreeCount);
  std::vector< std::pair< uint, uint > > stack;
  for (uint root = 0; root < octreeCount; ++root) {
    if (state[root] != 0) {
      continue;
    }
    state[root] = 1;
    stack.emplace_back(root, 0);
    while (!stack.empty()) {
      auto& [index, next] = stack.back();
      const auto& node = mOctree[index];
      if (node.mIsLeaf || next == 8) {
        state[index] = 2;
        stack.pop_back();
        continue;
      }
      const uint child = node.mChildren[next++] - mOctree.data();
      REQUIRE(state[child] != 1, "Cycle in PATH octree at node {}", child);
      if (state[child] == 0) {
        state[child] = 1;
        stack.emplace_back(child, 0);
      }
    }
  }
}
