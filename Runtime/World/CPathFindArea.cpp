#include "Runtime/World/CPathFindArea.hpp"

#include <logvisor/logvisor.hpp>

#include "Runtime/CToken.hpp"
#include "Runtime/IVParamObj.hpp"

namespace metaforce {

static logvisor::Module Log("CPathFindArea");

CPFAreaOctree::CPFAreaOctree(CMemoryInStream& in) {
  x0_isLeaf = in.ReadLong();
  x4_aabb = in.Get<zeus::CAABox>();
  x1c_center = in.Get<zeus::CVector3f>();
  for (auto& ptr : x28_children)
    ptr = reinterpret_cast<CPFAreaOctree*>(in.ReadLong());
  x48_regions.set_size(in.ReadLong());
  x48_regions.set_data(reinterpret_cast<CPFRegion**>(in.ReadLong()));
}

void CPFAreaOctree::Fixup(CPFArea& area) {
  x0_isLeaf = x0_isLeaf != 0 ? 1 : 0;
  if (x0_isLeaf) {
    if (x48_regions.empty())
      return;
    x48_regions.set_data(&area.x160_octreeRegionLookup[reinterpret_cast<uintptr_t>(x48_regions.data())]);
    return;
  }

  for (auto& ptr : x28_children) {
    if ((reinterpret_cast<uintptr_t>(ptr) & 0x80000000) == 0)
      ptr = &area.x158_octree[reinterpret_cast<uintptr_t>(ptr)];
    else
      ptr = nullptr;
  }
}

u32 CPFAreaOctree::GetChildIndex(const zeus::CVector3f& point) const {
  u32 idx = 0x0u;
  if (point.x() > x1c_center.x())
    idx = 0x1u;
  if (point.y() > x1c_center.y())
    idx |= 0x2u;
  if (point.z() > x1c_center.z())
    idx |= 0x4u;
  return idx;
}

rstl::prereserved_vector<CPFRegion*>* CPFAreaOctree::GetRegionList(const zeus::CVector3f& point) {
  if (x0_isLeaf != 0u) {
    return &x48_regions;
  }
  return x28_children[GetChildIndex(point)]->GetRegionList(point);
}

void CPFAreaOctree::GetRegionListList(rstl::reserved_vector<rstl::prereserved_vector<CPFRegion*>*, 32>& listOut,
                                      const zeus::CVector3f& point, float padding) {
  if (listOut.size() >= listOut.capacity())
    return;

  if (x0_isLeaf) {
    listOut.push_back(&x48_regions);
    return;
  }

  for (CPFAreaOctree* ch : x28_children) {
    if (ch->IsPointInsidePaddedAABox(point, padding))
      ch->GetRegionListList(listOut, point, padding);
  }
}

bool CPFAreaOctree::IsPointInsidePaddedAABox(const zeus::CVector3f& point, float padding) const {
  return point.x() >= x4_aabb.min.x() - padding && point.x() <= x4_aabb.max.x() + padding &&
         point.y() >= x4_aabb.min.y() - padding && point.y() <= x4_aabb.max.y() + padding &&
         point.z() >= x4_aabb.min.z() - padding && point.z() <= x4_aabb.max.z() + padding;
}

CPFOpenList::CPFOpenList() {
  x40_region.SetData(&x90_regionData);
  Clear();
}

void CPFOpenList::Clear() {
  x40_region.Data()->SetOpenMore(&x40_region);
  x40_region.Data()->SetOpenLess(&x40_region);
  x0_bitSet.Clear();
}

void CPFOpenList::Push(CPFRegion* reg) {
  x0_bitSet.Add(reg->GetIndex());
  CPFRegion* other = x40_region.Data()->GetOpenMore();
  while (other != &x40_region && reg->Data()->GetCost() > other->Data()->GetCost()) {
    other = other->Data()->GetOpenMore();
  }
  other->Data()->GetOpenLess()->Data()->SetOpenMore(reg);
  reg->Data()->SetOpenLess(other->Data()->GetOpenLess());
  other->Data()->SetOpenLess(reg);
  reg->Data()->SetOpenMore(other);
}

CPFRegion* CPFOpenList::Pop() {
  CPFRegion* reg = x40_region.Data()->GetOpenMore();
  if (reg != &x40_region) {
    x0_bitSet.Rmv(reg->GetIndex());
    reg->Data()->GetOpenMore()->Data()->SetOpenLess(reg->Data()->GetOpenLess());
    reg->Data()->GetOpenLess()->Data()->SetOpenMore(reg->Data()->GetOpenMore());
    reg->Data()->SetOpenMore(nullptr);
    reg->Data()->SetOpenLess(nullptr);
    return reg;
  }
  return nullptr;
}

void CPFOpenList::Pop(CPFRegion* reg) {
  x0_bitSet.Rmv(reg->GetIndex());
  reg->Data()->GetOpenMore()->Data()->SetOpenLess(reg->Data()->GetOpenLess());
  reg->Data()->GetOpenLess()->Data()->SetOpenMore(reg->Data()->GetOpenMore());
  reg->Data()->SetOpenMore(nullptr);
  reg->Data()->SetOpenLess(nullptr);
}

bool CPFOpenList::Test(const CPFRegion* reg) const { return x0_bitSet.Test(reg->GetIndex()); }

CPFArea::CPFArea(std::unique_ptr<u8[]>&& buf, u32 len) {
  CMemoryInStream r(buf.get(), len);

  u32 version = r.ReadLong();
  if (version != 4)
    Log.report(logvisor::Fatal, FMT_STRING("Unexpected PATH version {}, should be 4"), version);

  u32 numNodes = r.ReadLong();
  x140_nodes.reserve(numNodes);
  for (u32 i = 0; i < numNodes; ++i)
    x140_nodes.emplace_back(r);

  u32 numLinks = r.ReadLong();
  x148_links.reserve(numLinks);
  for (u32 i = 0; i < numLinks; ++i)
    x148_links.emplace_back(r);

  u32 numRegions = r.ReadLong();
  x150_regions.reserve(numRegions);
  for (u32 i = 0; i < numRegions; ++i)
    x150_regions.emplace_back(r);

  x178_regionDatas.resize(numRegions);

  u32 maxRegionNodes = 0;
  for (CPFRegion& region : x150_regions)
    region.Fixup(*this, maxRegionNodes);
  maxRegionNodes = std::max(maxRegionNodes, 4u);

  x10_tmpPolyPoints.reserve(maxRegionNodes);

  u32 numBitfieldWords = (numRegions * (numRegions - 1) / 2 + 31) / 32;
  x168_connectionsGround.reserve(numBitfieldWords);
  for (u32 i = 0; i < numBitfieldWords; ++i)
    x168_connectionsGround.push_back(r.ReadLong());
  x170_connectionsFlyers.reserve(numBitfieldWords);
  for (u32 i = 0; i < numBitfieldWords; ++i)
    x170_connectionsFlyers.push_back(r.ReadLong());

  for (u32 i = 0; i < ((((numRegions * numRegions) + 31) / 32) - numBitfieldWords) * 2 * sizeof(u32); ++i) {
    r.ReadChar();
  }

  u32 numRegionLookups = r.ReadLong();
  x160_octreeRegionLookup.reserve(numRegionLookups);
  for (u32 i = 0; i < numRegionLookups; ++i)
    x160_octreeRegionLookup.push_back(reinterpret_cast<CPFRegion*>(r.ReadLong()));

  for (CPFRegion*& rl : x160_octreeRegionLookup)
    rl = &x150_regions[reinterpret_cast<uintptr_t>(rl)];

  u32 numOctreeNodes = r.ReadLong();
  x158_octree.reserve(numOctreeNodes);
  for (u32 i = 0; i < numOctreeNodes; ++i)
    x158_octree.emplace_back(r);

  for (CPFAreaOctree& node : x158_octree)
    node.Fixup(*this);
}

rstl::prereserved_vector<CPFRegion*>* CPFArea::GetOctreeRegionList(const zeus::CVector3f& point) {
  if (x30_hasCachedRegionList && zeus::close_enough(point, x24_cachedRegionListPoint))
    return x20_cachedRegionList;
  return x158_octree.back().GetRegionList(point);
}

u32 CPFArea::FindRegions(rstl::reserved_vector<CPFRegion*, 4>& regions, const zeus::CVector3f& point, u32 flags,
                         u32 indexMask) {
  bool isFlyer = (flags & 0x2u) != 0;
  bool isSwimmer = (flags & 0x4u) != 0;
  for (CPFRegion* region : *GetOctreeRegionList(point)) {
    if (region->GetFlags() & 0xffu & flags && (region->GetFlags() >> 16u) & 0xffu & indexMask &&
        region->IsPointInside(point) && (isFlyer || isSwimmer || region->PointHeight(point) < 3.f)) {
      regions.push_back(region);
      if (regions.size() == regions.capacity())
        break;
    }
  }
  return u32(regions.size());
}

CPFRegion* CPFArea::FindClosestRegion(const zeus::CVector3f& point, u32 flags, u32 indexMask, float padding) {
  rstl::reserved_vector<rstl::prereserved_vector<CPFRegion*>*, 32> regionListList;
  x158_octree.back().GetRegionListList(regionListList, point, padding);
  bool isFlyer = (flags & 0x2u) != 0;
  CPFRegion* ret = nullptr;
  for (rstl::prereserved_vector<CPFRegion*>* list : regionListList) {
    for (CPFRegion* region : *list) {
      if (region->Data()->GetCookie() != x34_regionFindCookie) {
        if (region->GetFlags() & 0xffu & flags && (region->GetFlags() >> 16u) & 0xffu & indexMask &&
            region->IsPointInsidePaddedAABox(point, padding) && (isFlyer || region->PointHeight(point) < 3.f)) {
          if (region->FindBestPoint(x10_tmpPolyPoints, point, flags, padding * padding)) {
            ret = region;
            padding = region->Data()->GetBestPointDistanceSquared() == 0.0
                          ? 0.f
                          : std::sqrt(region->Data()->GetBestPointDistanceSquared());
            x4_closestPoint = region->Data()->GetBestPoint();
          }
        }
        region->Data()->SetCookie(x34_regionFindCookie);
      }
    }
  }
  ++x34_regionFindCookie;
  return ret;
}

zeus::CVector3f CPFArea::FindClosestReachablePoint(rstl::reserved_vector<CPFRegion*, 4>& regs,
                                                   const zeus::CVector3f& point, u32 flags, u32 indexMask) {
  zeus::CVector3f ret;
  float closestDistSq = FLT_MAX;
  for (CPFRegion& reg : x150_regions) {
    if (reg.GetFlags() & 0xffu & flags && (reg.GetFlags() >> 16u) & 0xffu & indexMask) {
      for (CPFRegion* oreg : regs) {
        if (PathExists(oreg, &reg, flags)) {
          float distSq = (reg.GetCentroid() - point).magSquared();
          if (distSq < closestDistSq) {
            closestDistSq = distSq;
            ret = reg.GetCentroid();
            break;
          }
        }
      }
    }
  }
  return ret;
}

bool CPFArea::PathExists(const CPFRegion* r1, const CPFRegion* r2, u32 flags) const {
  if (r1 == r2 || (flags & 0x4u) != 0)
    return true;

  u32 i1 = r1->GetIndex();
  u32 i2 = r2->GetIndex();
  if (i1 > i2)
    std::swap(i1, i2);

  u32 remRegions = u32(x150_regions.size()) - i1;
  u32 remConnections = remRegions * (remRegions - 1) / 2;
  u32 totalConnections = u32(x150_regions.size()) * (u32(x150_regions.size()) - 1) / 2;
  u32 bit = totalConnections - remConnections + i2 - (i1 + 1);

  auto d = std::div(bit, 32);
  if ((flags & 0x2u) != 0)
    return ((x170_connectionsFlyers[d.quot] >> u32(d.rem)) & 0x1u) != 0;
  else
    return ((x168_connectionsGround[d.quot] >> u32(d.rem)) & 0x1u) != 0;
}

CFactoryFnReturn FPathFindAreaFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                      const metaforce::CVParamTransfer& vparms, CObjectReference*) {
  return TToken<CPFArea>::GetIObjObjectFor(std::make_unique<CPFArea>(std::move(in), len));
}
} // namespace metaforce
