#include "MetroidPrime/PathFinding/CPathFindArea.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"

#include "rstl/algorithm.hpp"

#include <dolphin/os.h>
#include <float.h>

class CVParamTransfer;

#if !defined(TARGET_PC)
class CPFMemoryStream {
public:
  CPFMemoryStream(uchar* data, int size) : x0_data(data), x4_size(size) {}
  int ReadInt32() {
    int value = *reinterpret_cast< int* >(x0_data);
    x0_data += sizeof(int);
    return value;
  }
  void* GetBlock(int count, int size) {
    void* block = x0_data;
    x0_data += count * size;
    return block;
  }

private:
  uchar* x0_data;
  int x4_size;
};
#endif

uint CPFAreaOctree::GetChildIndex(const CVector3f& point) const {
  uint index = 0;
  if (point[kDX] > x1c_center[kDX]) {
    index = 1;
  }
  if (point[kDY] > x1c_center[kDY]) {
    index |= 2;
  }
  if (point[kDZ] > x1c_center[kDZ]) {
    index |= 4;
  }
  return index;
}

prereserved_vector< CPFRegion* >* CPFAreaOctree::GetRegionList(const CVector3f& point) {
  if (x0_isLeaf) {
    return &x48_regions;
  }
  return x28_children[GetChildIndex(point)]->GetRegionList(point);
}

void CPFAreaOctree::GetRegionListList(
    rstl::reserved_vector< prereserved_vector< CPFRegion* >*, 32 >& lists, const CVector3f& point,
    float padding) {
  if (lists.size() >= lists.capacity()) {
    return;
  }
  if (x0_isLeaf) {
    lists.push_back(&x48_regions);
  } else {
    for (int i = 0; i < 8; ++i) {
      if (x28_children[i]->IsPointInsidePaddedAABox(point, padding)) {
        x28_children[i]->GetRegionListList(lists, point, padding);
      }
    }
  }
}

CPFArea::CPFArea(const rstl::auto_ptr< uchar >& data, int size)
: x0_bestPointDistSq(FLT_MAX)
, x4_closestPoint(CVector3f::Zero())
, x20_cachedRegionList(nullptr)
, x24_cachedRegionListPoint(CVector3f::Zero())
, x30_hasCachedRegionList(false)
, x34_regionFindCookie(0)
#if !defined(TARGET_PC)
, x13c_data(data.release())
#endif
, x188_transform(CTransform4f::Identity()) {
#if defined(TARGET_PC)
  rstl::single_ptr resource{data.release()};
  ReadData(size < 0 || resource.null() ? std::span< const uchar >{}
                                       : std::span< const uchar >(resource.get(), size));
#else
  CPFMemoryStream stream(x13c_data.get(), size);
  stream.ReadInt32();

  int numNodes = stream.ReadInt32();
  x140_nodes.set_size(numNodes);
  x140_nodes.set_data(static_cast< CPFNode* >(stream.GetBlock(numNodes, sizeof(CPFNode))));
  int numLinks = stream.ReadInt32();
  x148_links.set_size(numLinks);
  x148_links.set_data(static_cast< CPFLink* >(stream.GetBlock(numLinks, sizeof(CPFLink))));
  const int numRegions = stream.ReadInt32();
  x150_regions.set_size(numRegions);
  x150_regions.set_data(static_cast< CPFRegion* >(stream.GetBlock(numRegions, sizeof(CPFRegion))));
  x178_regionData.reserve(numRegions);
  CPFRegionData dataValue = CPFRegionData();
  x178_regionData.resize(numRegions, dataValue);
  int maxRegionNodes = 0;
  int i;
  for (i = 0; i < numRegions; ++i) {
    x150_regions[i].Fixup(*this, maxRegionNodes);
  }
  maxRegionNodes = maxRegionNodes > 4 ? maxRegionNodes : 4;
  x10_polyPoints.reserve(maxRegionNodes);

  int numWords = (numRegions * (numRegions - 1) / 2 + 31) / 32;
  x168_connectionsGround.set_size(numWords);
  x168_connectionsGround.set_data(static_cast< uint* >(stream.GetBlock(numWords, sizeof(uint))));
  x170_connectionsFlyers.set_size(numWords);
  x170_connectionsFlyers.set_data(static_cast< uint* >(stream.GetBlock(numWords, sizeof(uint))));
  stream.GetBlock(((numRegions * numRegions + 31) / 32 - numWords) * 2, sizeof(uint));

  int numRegionPtrs = stream.ReadInt32();
  x160_octreeRegions.set_size(numRegionPtrs);
  x160_octreeRegions.set_data(
      static_cast< CPFRegion** >(stream.GetBlock(numRegionPtrs, sizeof(CPFRegion*))));
  for (i = 0; i < numRegionPtrs; ++i) {
    CPFRegion* const& region = x160_octreeRegions[i];
    x160_octreeRegions[i] = &x150_regions[reinterpret_cast< intptr_t >(region)];
  }
  int numOctreeNodes = stream.ReadInt32();
  x158_octree.set_size(numOctreeNodes);
  x158_octree.set_data(
      static_cast< CPFAreaOctree* >(stream.GetBlock(numOctreeNodes, sizeof(CPFAreaOctree))));
  for (i = 0; i < numOctreeNodes; ++i) {
    x158_octree[i].Fixup(*this);
  }
#endif
}

prereserved_vector< CPFRegion* >* CPFArea::GetOctreeRegionList(const CVector3f& point) {
  if (x30_hasCachedRegionList && close_enough(point, x24_cachedRegionListPoint)) {
    return x20_cachedRegionList;
  }
  return x158_octree.back().GetRegionList(point);
}

int CPFArea::FindRegions(rstl::reserved_vector< CPFRegion*, 4 >& regions, const CVector3f& point,
                         uint flags, uint indexMask) {
  prereserved_vector< CPFRegion* >* list = GetOctreeRegionList(point);
  for (int i = 0; i < list->size(); ++i) {
    CPFRegion* region = (*list)[i];
    if ((region->GetFlags() & 0xff & flags) && ((region->GetFlags() >> 16) & 0xff & indexMask) &&
        region->IsPointInside(point) &&
        ((flags & 2) || (flags & 4) || region->PointHeight(point) < 3.f)) {
      regions.push_back(region);
      if (regions.size() == regions.capacity()) {
        break;
      }
    }
  }
  return regions.size();
}

CPFRegion* CPFArea::FindClosestRegion(const CVector3f& point, uint flags, uint indexMask,
                                      float padding) {
  rstl::reserved_vector< prereserved_vector< CPFRegion* >*, 32 > lists;
  CPFRegion* result = nullptr;
  OSGetTick();
  int i, j;
  uint searchTicks = 0;
  x158_octree.back().GetRegionListList(lists, point, padding);
  OSGetTick();
  for (i = 0; i < lists.size(); ++i) {
    prereserved_vector< CPFRegion* >* list = lists[i];
    for (j = 0; j < list->size(); ++j) {
      CPFRegion* region = (*list)[j];
      if (region->Data()->GetCookie() != x34_regionFindCookie) {
        if ((region->GetFlags() & 0xff & flags) &&
            ((region->GetFlags() >> 16) & 0xff & indexMask) &&
            region->IsPointInsidePaddedAABox(point, padding)) {
          uint startTick = OSGetTick();
          if ((flags & 2) || region->PointHeight(point) < 3.f) {
            if (region->FindBestPoint(x10_polyPoints, point, flags, padding * padding)) {
              padding = CMath::FastSqrtF(region->Data()->GetBestDistanceSquared());
              result = region;
              x4_closestPoint = region->Data()->GetBestPoint();
            }
            searchTicks += OSGetTick() - startTick;
          }
        }
        region->Data()->SetCookie(x34_regionFindCookie);
      }
    }
  }
  OSGetTick();
  ++x34_regionFindCookie;
  return result;
}

CVector3f CPFArea::FindClosestReachablePoint(rstl::reserved_vector< CPFRegion*, 4 >& regions,
                                             const CVector3f& point, uint flags, uint indexMask) {
  CVector3f result = CVector3f::Zero();
  float closestDistanceSq = FLT_MAX;
  for (int i = 0; i < GetNumRegions(); ++i) {
    CPFRegion& region = GetRegion(i);
    if ((region.GetFlags() & 0xff & flags) && ((region.GetFlags() >> 16) & 0xff & indexMask)) {
      for (int j = 0; j < regions.size(); ++j) {
        CPFRegion* source = regions[j];
        if (PathExists(source, &region, flags)) {
          const CVector3f& delta = region.GetCentroid() - point;
          float distanceSq = delta.MagSquared();
          if (distanceSq < closestDistanceSq) {
            closestDistanceSq = distanceSq;
            result = region.GetCentroid();
            break;
          }
        }
      }
    }
  }
  return result;
}

bool CPFArea::PathExists(const CPFRegion* source, const CPFRegion* destination, uint flags) const {
  if (source == destination || (flags & 4)) {
    return true;
  }
  int numRegions = GetNumRegions();
  int sourceIndex = source->GetIndex();
  int destinationIndex = destination->GetIndex();
  if (sourceIndex > destinationIndex) {
    rstl::swap(sourceIndex, destinationIndex);
  }
  int totalConnections = numRegions * (numRegions - 1) / 2;
  int remainingConnections = (numRegions - sourceIndex - 1) * (numRegions - sourceIndex) / 2;
  uint bit = totalConnections - remainingConnections + destinationIndex - (sourceIndex + 1);
  if (flags & 2) {
    return (x170_connectionsFlyers[bit / 32] >> (bit % 32)) & 1;
  }
  return (x168_connectionsGround[bit / 32] >> (bit % 32)) & 1;
}

const CFactoryFnReturn FPathFindAreaFactory(const SObjectTag& tag,
                                            const rstl::auto_ptr< uchar >& data, int size,
                                            const CVParamTransfer& xfer) {
  return rs_new CPFArea(data, size);
}
