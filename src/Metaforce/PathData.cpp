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

  x140_nodes.resize(in.ReadCount(24), CPFNode());
  for (auto& node : x140_nodes) {
    node.x0_position = in.ReadVector3f();
    node.xc_normal = in.ReadVector3f();
  }
  x148_links.resize(in.ReadCount(16), CPFLink());
  for (auto& link : x148_links) {
    link.x0_node = in.Read< int >();
    link.x4_region = in.Read< int >();
    link.x8_2dWidth = in.Read< float >();
    link.xc_oo2dWidth = in.Read< float >();
  }

  const uint regionCount = in.ReadCount(80);
  REQUIRE(regionCount <= 512, "PATH has {} regions; search bitsets hold 512", regionCount);
  x150_regions.resize(regionCount, CPFRegion());
  x178_regionData.resize(regionCount, CPFRegionData());
  std::array< bool, 512 > regionIndices{};
  int maxRegionNodes = 4;
  for (auto& region : x150_regions) {
    const uint nodeCount = in.Read< uint >();
    const uint firstNode = in.Read< uint >();
    const uint linkCount = in.Read< uint >();
    const uint firstLink = in.Read< uint >();
    REQUIRE(nodeCount != 0, "PATH region has no polygon nodes");
    CheckRange(firstNode, nodeCount, x140_nodes.size());
    if (linkCount != 0) {
      CheckRange(firstLink, linkCount, x148_links.size());
    }
    region.x0_numNodes = nodeCount;
    region.x4_startNode = x140_nodes.data() + firstNode;
    region.x8_numLinks = linkCount;
    region.xc_startLink = linkCount ? x148_links.data() + firstLink : nullptr;
    region.x10_flags = in.Read< uint >();
    region.x14_height = in.Read< float >();
    region.x18_normal = in.ReadVector3f();
    const uint index = in.Read< uint >();
    REQUIRE(index < regionCount && !regionIndices[index], "Invalid PATH region index {}", index);
    regionIndices[index] = true;
    region.x24_regionIdx = index;
    region.x28_centroid = in.ReadVector3f();
    region.x34_bounds = in.ReadAABox();
    in.Read< uint >(); // The serialized scratch-data pointer is not persistent data.
    region.x4c_data = &x178_regionData[index];
    maxRegionNodes = std::max(maxRegionNodes, region.x0_numNodes);
    for (uint i = 0; i < linkCount; ++i) {
      const auto& link = region.xc_startLink[i];
      REQUIRE(link.GetNode() >= 0 && static_cast< uint >(link.GetNode()) < nodeCount &&
                  link.GetRegion() >= 0 && static_cast< uint >(link.GetRegion()) < regionCount,
              "Invalid PATH link in region {}", index);
    }
  }
  x10_polyPoints.reserve(maxRegionNodes);

  const uint connectionWords = regionCount ? (regionCount * (regionCount - 1) / 2 + 31) / 32 : 0;
  x168_connectionsGround.reserve(connectionWords);
  x170_connectionsFlyers.reserve(connectionWords);
  for (uint i = 0; i < connectionWords; ++i) {
    x168_connectionsGround.push_back(in.Read< uint >());
  }
  for (uint i = 0; i < connectionWords; ++i) {
    x170_connectionsFlyers.push_back(in.Read< uint >());
  }
  // The file reserves two square bit matrices; the game uses triangular tables.
  in.Take(((regionCount * regionCount + 31) / 32 - connectionWords) * 2 * sizeof(uint));

  const uint lookupCount = in.ReadCount(4);
  x160_octreeRegions.reserve(lookupCount);
  for (uint i = 0; i < lookupCount; ++i) {
    const uint index = in.Read< uint >();
    REQUIRE(index < regionCount, "Invalid PATH octree region index {}", index);
    x160_octreeRegions.push_back(&x150_regions[index]);
  }
  const uint octreeCount = in.ReadCount(80);
  REQUIRE(octreeCount != 0, "PATH has no octree root");
  x158_octree.resize(octreeCount, CPFAreaOctree());
  for (auto& node : x158_octree) {
    node.x0_isLeaf = in.Read< uint >() != 0;
    node.x4_bounds = in.ReadAABox();
    node.x1c_center = in.ReadVector3f();
    for (auto& child : node.x28_children) {
      const uint index = in.Read< uint >();
      if (!node.x0_isLeaf) {
        // Traversal visits all eight children without testing for null.
        REQUIRE(index < octreeCount, "Invalid PATH octree child {}", index);
        child = &x158_octree[index];
      } else {
        child = nullptr;
      }
    }
    const uint count = in.Read< uint >();
    const uint firstRegion = in.Read< uint >();
    if (node.x0_isLeaf && count != 0) {
      CheckRange(firstRegion, count, lookupCount);
      node.x48_regions.set_size(count);
      node.x48_regions.set_data(x160_octreeRegions.data() + firstRegion);
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
      const auto& node = x158_octree[index];
      if (node.x0_isLeaf || next == 8) {
        state[index] = 2;
        stack.pop_back();
        continue;
      }
      const uint child = node.x28_children[next++] - x158_octree.data();
      REQUIRE(state[child] != 1, "Cycle in PATH octree at node {}", child);
      if (state[child] == 0) {
        state[child] = 1;
        stack.emplace_back(child, 0);
      }
    }
  }
}
