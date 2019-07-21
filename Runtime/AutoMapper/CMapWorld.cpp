#include "CMapWorld.hpp"
#include "CMapWorldInfo.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "World/CWorld.hpp"
#include "CStateManager.hpp"

namespace urde {

CMapWorld::CMapAreaData::CMapAreaData(CAssetId areaRes, EMapAreaList list, CMapAreaData* next)
: x0_area(g_SimplePool->GetObj(SObjectTag{FOURCC('MAPA'), areaRes})), x10_list(list), x14_next(next) {}

CMapWorld::CMapWorld(CInputStream& in) {
  x10_listHeads.resize(3);
  in.readUint32Big();
  in.readUint32Big();
  u32 areaCount = in.readUint32Big();
  x0_areas.reserve(areaCount);
  x20_traversed.resize(areaCount);
  for (u32 i = 0; i < areaCount; ++i) {
    CAssetId mapaId = in.readUint32Big();
    x0_areas.emplace_back(mapaId, EMapAreaList::Unloaded, x0_areas.empty() ? nullptr : &x0_areas.back());
  }
  x10_listHeads[2] = &x0_areas.back();
}

bool CMapWorld::IsMapAreaInBFSInfoVector(const CMapWorld::CMapAreaData* area,
                                         const std::vector<CMapWorld::CMapAreaBFSInfo>& vec) const {
  for (const CMapWorld::CMapAreaBFSInfo& bfs : vec) {
    if (&x0_areas[bfs.GetAreaIndex()] == area)
      return true;
  }
  return false;
}

void CMapWorld::SetWhichMapAreasLoaded(const IWorld& wld, int start, int count) {
  ClearTraversedFlags();

  std::vector<CMapAreaBFSInfo> bfsInfos;
  bfsInfos.reserve(x0_areas.size());
  DoBFS(wld, start, count, 9999.f, 9999.f, false, bfsInfos);

  for (int i = 0; i < 2; ++i) {
    for (CMapAreaData* data = x10_listHeads[i]; data;) {
      CMapAreaData* nextData = data->NextMapAreaData();
      if (!IsMapAreaInBFSInfoVector(data, bfsInfos)) {
        data->Unlock();
        MoveMapAreaToList(data, EMapAreaList::Unloaded);
      }
      data = nextData;
    }
  }

  for (CMapAreaBFSInfo& bfs : bfsInfos) {
    CMapAreaData& data = x0_areas[bfs.GetAreaIndex()];
    data.Lock();
    if (data.GetContainingList() == EMapAreaList::Unloaded)
      MoveMapAreaToList(&data, EMapAreaList::Loading);
  }
}

bool CMapWorld::IsMapAreasStreaming() const {
  bool ret = false;
  CMapAreaData* data = x10_listHeads[1];
  while (data != nullptr) {
    if (data->IsLoaded()) {
      CMapAreaData* next = data->NextMapAreaData();
      const_cast<CMapWorld*>(this)->MoveMapAreaToList(data, EMapAreaList::Loaded);
      data = next;
    } else {
      data = data->NextMapAreaData();
      ret = true;
    }
  }
  return ret;
}

void CMapWorld::MoveMapAreaToList(CMapWorld::CMapAreaData* data, CMapWorld::EMapAreaList list) {
  CMapAreaData* last = nullptr;
  for (CMapAreaData* head = x10_listHeads[int(data->GetContainingList())];;
       last = head, head = head->NextMapAreaData()) {
    if (head != data)
      continue;
    if (!last)
      x10_listHeads[int(data->GetContainingList())] = head->NextMapAreaData();
    else
      last->SetNextMapArea(head->NextMapAreaData());
    break;
  }
  data->SetNextMapArea(x10_listHeads[int(list)]);
  data->SetContainingList(list);
  x10_listHeads[int(list)] = data;
}

s32 CMapWorld::GetCurrentMapAreaDepth(const IWorld& wld, TAreaId aid) const {
  ClearTraversedFlags();
  std::vector<CMapAreaBFSInfo> info;
  info.reserve(x0_areas.size());
  DoBFS(wld, aid, 9999, 9999.f, 9999.f, false, info);
  if (info.empty())
    return 0;
  return info.back().GetDepth();
}

std::vector<int> CMapWorld::GetVisibleAreas(const IWorld& wld, const CMapWorldInfo& mwInfo) const {
  std::vector<int> ret;
  ret.reserve(x0_areas.size());
  for (size_t i = 0; i < x0_areas.size(); ++i) {
    if (!IsMapAreaValid(wld, i, true))
      continue;
    const CMapArea* area = GetMapArea(i);
    bool areaVis = mwInfo.IsAreaVisible(i);
    bool worldVis = mwInfo.IsWorldVisible(i);
    if (area->GetIsVisibleToAutoMapper(worldVis, areaVis))
      ret.push_back(i);
  }
  return ret;
}

void CMapWorld::Draw(const CMapWorld::CMapWorldDrawParms& parms, int curArea, int otherArea, float depth1, float depth2,
                     bool inMapScreen) const {
  if (depth1 == 0.f && depth2 == 0.f)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CMapWorld::Draw", zeus::skBlue);

  ClearTraversedFlags();
  int areaDepth = std::ceil(std::max(depth1, depth2));

  std::vector<CMapAreaBFSInfo> bfsInfos;
  bfsInfos.reserve(x0_areas.size());
  if (curArea != otherArea) {
    const_cast<CMapWorld*>(this)->x20_traversed[otherArea] = true;
    DoBFS(parms.GetWorld(), curArea, areaDepth, depth1, depth2, true, bfsInfos);

    float lowD1 = std::ceil(depth1 - 1.f);
    float tmp;
    if (depth1 == std::floor(depth1))
      tmp = 0.f;
    else
      tmp = 1.f - std::fmod(depth1, 1.f);
    float newD1 = lowD1 + tmp;

    float lowD2 = std::ceil(depth2 - 1.f);
    if (depth2 == std::floor(depth2))
      tmp = 0.f;
    else
      tmp = 1.f - std::fmod(depth2, 1.f);
    float newD2 = lowD2 + tmp;

    int otherDepth = std::ceil(std::max(newD1, newD2));
    if (parms.GetWorld().IGetAreaAlways(otherArea)->IIsActive()) {
      const_cast<CMapWorld*>(this)->x20_traversed[otherArea] = false;
      DoBFS(parms.GetWorld(), otherArea, otherDepth, newD1, newD2, true, bfsInfos);
    }
  } else {
    DoBFS(parms.GetWorld(), curArea, areaDepth, depth1, depth2, true, bfsInfos);
  }

  DrawAreas(parms, curArea, bfsInfos, inMapScreen);
}

void CMapWorld::DoBFS(const IWorld& wld, int startArea, int areaCount, float surfDepth, float outlineDepth,
                      bool checkLoad, std::vector<CMapAreaBFSInfo>& bfsInfos) const {
  if (areaCount <= 0 || !IsMapAreaValid(wld, startArea, checkLoad))
    return;

  size_t size = bfsInfos.size();
  bfsInfos.emplace_back(startArea, 1, surfDepth, outlineDepth);
  const_cast<CMapWorld*>(this)->x20_traversed[startArea] = true;

  for (; size != bfsInfos.size(); ++size) {
    CMapAreaBFSInfo& testInfo = bfsInfos[size];
    if (testInfo.GetDepth() == areaCount)
      continue;

    surfDepth = testInfo.GetSurfaceDrawDepth() - 1.f;
    outlineDepth = testInfo.GetOutlineDrawDepth() - 1.f;

    const IGameArea* area = wld.IGetAreaAlways(testInfo.GetAreaIndex());
    for (u32 i = 0; i < area->IGetNumAttachedAreas(); ++i) {
      TAreaId attId = area->IGetAttachedAreaId(i);
      if (IsMapAreaValid(wld, attId, checkLoad) && !x20_traversed[attId]) {
        bfsInfos.emplace_back(attId, testInfo.GetDepth() + 1, surfDepth, outlineDepth);
        const_cast<CMapWorld*>(this)->x20_traversed[attId] = true;
      }
    }
  }
}

bool CMapWorld::IsMapAreaValid(const IWorld& wld, int areaIdx, bool checkLoad) const {
  if (!wld.IGetAreaAlways(areaIdx)->IIsActive())
    return false;
  const CMapArea* mapa = GetMapArea(areaIdx);
  if (checkLoad)
    return mapa != nullptr;
  return true;
}

void CMapWorld::DrawAreas(const CMapWorld::CMapWorldDrawParms& parms, int selArea,
                          const std::vector<CMapAreaBFSInfo>& bfsInfos, bool inMapScreen) const {
  // Alpha blend
  // Line width 1

  int surfCount = 0;
  int objCount = 0;
  for (const CMapAreaBFSInfo& bfsInfo : bfsInfos) {
    const CMapArea* mapa = GetMapArea(bfsInfo.GetAreaIndex());
    surfCount += mapa->GetNumSurfaces();
    objCount += mapa->GetNumMappableObjects();
  }

  std::vector<CMapObjectSortInfo> sortInfos;
  sortInfos.reserve(surfCount + objCount + (parms.GetIsSortDoorSurfaces() ? objCount * 6 : 0));

  int playerArea = parms.GetStateManager().GetNextAreaId();
  const CMapWorldInfo& mwInfo = parms.GetMapWorldInfo();
  for (const CMapAreaBFSInfo& bfsInfo : bfsInfos) {
    int thisArea = bfsInfo.GetAreaIndex();
    const CMapArea* mapa = GetMapArea(thisArea);
    if (!mapa->GetIsVisibleToAutoMapper(mwInfo.IsWorldVisible(thisArea), mwInfo.IsAreaVisible(thisArea)))
      continue;

    float surfDepth = bfsInfo.GetSurfaceDrawDepth();
    float outlineDepth = bfsInfo.GetOutlineDrawDepth();

    if (surfDepth >= 1.f)
      surfDepth = 1.f;
    else if (surfDepth < 0.f)
      surfDepth = 0.f;
    else
      surfDepth -= std::floor(surfDepth);

    if (outlineDepth >= 1.f)
      outlineDepth = 1.f;
    else if (outlineDepth < 0.f)
      outlineDepth = 0.f;
    else
      outlineDepth -= std::floor(outlineDepth);

    float alphaSurf;
    float alphaOutline;
    const zeus::CColor* surfaceColor;
    const zeus::CColor* outlineColor;
    const zeus::CColor* surfacePlayerColor;
    const zeus::CColor* outlinePlayerColor;
    if (mwInfo.IsAreaVisted(thisArea)) {
      alphaSurf = parms.GetAlphaSurfaceVisited();
      alphaOutline = parms.GetAlphaOutlineVisited();
      surfaceColor = &g_tweakAutoMapper->GetSurfaceVisitedColor();
      outlineColor = &g_tweakAutoMapper->GetOutlineVisitedColor();
      surfacePlayerColor = &g_tweakAutoMapper->GetSurfaceSelectVisitedColor();
      outlinePlayerColor = &g_tweakAutoMapper->GetOutlineSelectVisitedColor();
    } else {
      alphaSurf = parms.GetAlphaSurfaceUnvisited();
      alphaOutline = parms.GetAlphaOutlineUnvisited();
      surfaceColor = &g_tweakAutoMapper->GetSurfaceUnvisitedColor();
      outlineColor = &g_tweakAutoMapper->GetOutlineUnvisitedColor();
      surfacePlayerColor = &g_tweakAutoMapper->GetSurfaceSelectUnvisitedColor();
      outlinePlayerColor = &g_tweakAutoMapper->GetOutlineSelectUnvisitedColor();
    }

    zeus::CColor hintFlashColor =
        zeus::CColor::lerp(zeus::skClear, zeus::CColor{1.f, 1.f, 1.f, 0.f}, parms.GetHintAreaFlashIntensity());

    zeus::CColor finalSurfColor, finalOutlineColor;
    if (thisArea == selArea && inMapScreen) {
      finalSurfColor = *surfacePlayerColor + hintFlashColor;
      finalOutlineColor = *outlinePlayerColor + hintFlashColor;
    } else {
      finalSurfColor = *surfaceColor;
      finalSurfColor.a() = surfDepth * alphaSurf;
      finalOutlineColor = *outlineColor;
      finalOutlineColor.a() = outlineDepth * alphaOutline;
    }

    if ((selArea != playerArea || parms.GetHintAreaFlashIntensity() == 0.f) && playerArea == thisArea &&
        this == parms.GetStateManager().GetWorld()->GetMapWorld()) {
      float pulse = parms.GetPlayerAreaFlashIntensity();
      const zeus::CColor& flashCol = g_tweakAutoMapper->GetAreaFlashPulseColor();
      finalSurfColor = zeus::CColor::lerp(finalSurfColor, flashCol, pulse);
      finalOutlineColor = zeus::CColor::lerp(finalOutlineColor, flashCol, pulse);
    }

    zeus::CTransform modelView =
        parms.GetCameraTransform().inverse() * mapa->GetAreaPostTransform(parms.GetWorld(), thisArea);
    for (u32 i = 0; i < mapa->GetNumSurfaces(); ++i) {
      const CMapArea::CMapAreaSurface& surf = mapa->GetSurface(i);
      zeus::CVector3f pos = modelView * surf.GetCenterPosition();
      sortInfos.emplace_back(pos.y(), thisArea, CMapObjectSortInfo::EObjectCode::Surface, i, finalSurfColor,
                             finalOutlineColor);
    }

    u32 i = 0;
    u32 si = 0;
    for (; i < mapa->GetNumMappableObjects(); ++i, si += 6) {
      const CMappableObject& obj = mapa->GetMappableObject(i);
      if (!obj.IsVisibleToAutoMapper(mwInfo.IsWorldVisible(thisArea), mwInfo))
        continue;

      bool doorType = CMappableObject::IsDoorType(obj.GetType());
      if (doorType) {
        if (!mwInfo.IsAreaVisible(thisArea))
          continue;
        if (parms.GetIsSortDoorSurfaces()) {
          for (u32 s = 0; s < 6; ++s) {
            zeus::CVector3f center = obj.BuildSurfaceCenterPoint(s);
            zeus::CVector3f pos = modelView * (CMapArea::GetAreaPostTranslate(parms.GetWorld(), thisArea) + center);
            sortInfos.emplace_back(pos.y(), thisArea, CMapObjectSortInfo::EObjectCode::DoorSurface, si + s,
                                   zeus::CColor{1.f, 0.f, 1.f, 1.f}, zeus::CColor{1.f, 0.f, 1.f, 1.f});
          }
          continue;
        }
      }

      zeus::CVector3f pos =
          modelView * (obj.GetTransform().origin + CMapArea::GetAreaPostTranslate(parms.GetWorld(), thisArea));
      sortInfos.emplace_back(pos.y(), thisArea,
                             doorType ? CMapObjectSortInfo::EObjectCode::Door : CMapObjectSortInfo::EObjectCode::Object,
                             i, zeus::CColor{1.f, 0.f, 1.f, 1.f}, zeus::CColor{1.f, 0.f, 1.f, 1.f});
    }
  }

  std::sort(sortInfos.begin(), sortInfos.end(), [](const CMapObjectSortInfo& a, const CMapObjectSortInfo& b) {
    return a.GetZDistance() > b.GetZDistance();
  });

  u32 lastAreaIdx = UINT32_MAX;
  CMapObjectSortInfo::EObjectCode lastType = CMapObjectSortInfo::EObjectCode::Invalid;
  for (const CMapObjectSortInfo& info : sortInfos) {
    const CMapArea* mapa = GetMapArea(info.GetAreaIndex());
    zeus::CTransform areaPostXf = mapa->GetAreaPostTransform(parms.GetWorld(), info.GetAreaIndex());
    if (info.GetObjectCode() == CMapObjectSortInfo::EObjectCode::Surface) {
      const CMapArea::CMapAreaSurface& surf = mapa->GetSurface(info.GetLocalObjectIndex());
      zeus::CColor color(
          std::max(0.f, (-parms.GetCameraTransform().basis[1]).dot(areaPostXf.rotate(surf.GetNormal()))) *
              g_tweakAutoMapper->GetMapSurfaceNormColorLinear() +
          g_tweakAutoMapper->GetMapSurfaceNormColorConstant());
      color *= info.GetSurfaceColor();
      if (lastAreaIdx != info.GetAreaIndex() || lastType != CMapObjectSortInfo::EObjectCode::Surface)
        CGraphics::SetModelMatrix(parms.GetPlaneProjectionTransform() * areaPostXf);
      surf.Draw(mapa->GetVertices(), color, info.GetOutlineColor(), parms.GetOutlineWidthScale());

      lastAreaIdx = info.GetAreaIndex();
      lastType = info.GetObjectCode();
    }
  }
  for (const CMapObjectSortInfo& info : sortInfos) {
    const CMapArea* mapa = GetMapArea(info.GetAreaIndex());
    if (info.GetObjectCode() == CMapObjectSortInfo::EObjectCode::Door ||
        info.GetObjectCode() == CMapObjectSortInfo::EObjectCode::Object) {
      const CMappableObject& mapObj = mapa->GetMappableObject(info.GetLocalObjectIndex());
      zeus::CTransform objXf =
          zeus::CTransform::Translate(CMapArea::GetAreaPostTranslate(parms.GetWorld(), info.GetAreaIndex())) *
          mapObj.GetTransform();
      if (info.GetObjectCode() == CMapObjectSortInfo::EObjectCode::Door) {
        CGraphics::SetModelMatrix(parms.GetPlaneProjectionTransform() * objXf);
      } else {
        CGraphics::SetModelMatrix(
            parms.GetPlaneProjectionTransform() * objXf *
            zeus::CTransform(parms.GetCameraTransform().buildMatrix3f() * zeus::CMatrix3f(parms.GetObjectScale())));
      }
      mapObj.Draw(selArea, mwInfo, parms.GetAlpha(), lastType != info.GetObjectCode());
      lastType = info.GetObjectCode();
    } else if (info.GetObjectCode() == CMapObjectSortInfo::EObjectCode::DoorSurface) {
      const CMappableObject& mapObj = mapa->GetMappableObject(info.GetLocalObjectIndex() / 6);
      zeus::CTransform objXf =
          parms.GetPlaneProjectionTransform() *
          zeus::CTransform::Translate(CMapArea::GetAreaPostTranslate(parms.GetWorld(), info.GetAreaIndex())) *
          mapObj.GetTransform();
      CGraphics::SetModelMatrix(objXf);
      mapObj.DrawDoorSurface(selArea, mwInfo, parms.GetAlpha(), info.GetLocalObjectIndex() % 6,
                             lastType != info.GetObjectCode());
      lastType = info.GetObjectCode();
    }
  }
}

struct Support {
  int x0_;
  int x4_[3];
};

struct Circle2 {
  zeus::CVector2f x0_point;
  float x8_radiusSq;
};

struct Circle {
  zeus::CVector2f x0_point;
  float x8_radius;
  Circle(const Circle2& circ2) : x0_point(circ2.x0_point), x8_radius(std::sqrt(circ2.x8_radiusSq)) {}
};

static Circle2 ExactCircle1(const zeus::CVector2f* a) { return {*a, 0.f}; }

static Circle2 ExactCircle2(const zeus::CVector2f* a, const zeus::CVector2f* b) {
  Circle2 ret = {};
  ret.x0_point = 0.5f * (*a + *b);
  ret.x8_radiusSq = (*b - *a).magSquared() * 0.25f;
  return ret;
}

static Circle2 ExactCircle3(const zeus::CVector2f* a, const zeus::CVector2f* b, const zeus::CVector2f* c) {
  Circle2 ret = {};
  zeus::CVector2f d1 = *b - *a;
  zeus::CVector2f d2 = *c - *a;
  float cross = d1.cross(d2);
  zeus::CVector2f magVec(d1.magSquared() * 0.5f, d2.magSquared() * 0.5f);
  if (std::fabs(cross) > 0.01f) {
    zeus::CVector2f tmp((d2.y() * magVec.x() - d1.y() * magVec.y()) / cross,
                        (d1.x() * magVec.y() - d2.x() * magVec.x()) / cross);
    ret.x0_point = *a + tmp;
    ret.x8_radiusSq = tmp.magSquared();
  } else {
    ret.x8_radiusSq = FLT_MAX;
  }
  return ret;
}

static bool PointInsideCircle(const zeus::CVector2f& point, const Circle2& circ, float& intersect) {
  intersect = (point - circ.x0_point).magSquared() - circ.x8_radiusSq;
  return intersect <= 0.f;
}

static Circle2 UpdateSupport1(int idx, const zeus::CVector2f** list, Support& support) {
  Circle2 ret = ExactCircle2(list[support.x4_[0]], list[idx]);
  support.x0_ = 2;
  support.x4_[1] = idx;
  return ret;
}

static Circle2 UpdateSupport2(int idx, const zeus::CVector2f** list, Support& support) {
  Circle2 circs[3] = {};
  float intersect;
  int circIdx = -1;
  float minRad = FLT_MAX;

  circs[0] = ExactCircle2(list[support.x4_[0]], list[idx]);
  if (PointInsideCircle(*list[support.x4_[1]], circs[0], intersect)) {
    minRad = circs[0].x8_radiusSq;
    circIdx = 0;
  }

  circs[1] = ExactCircle2(list[support.x4_[1]], list[idx]);
  if (circs[1].x8_radiusSq < minRad && PointInsideCircle(*list[support.x4_[0]], circs[1], intersect)) {
    circIdx = 1;
  }

  Circle2 ret;
  if (circIdx != -1) {
    ret = circs[circIdx];
    support.x4_[1 - circIdx] = idx;
  } else {
    ret = ExactCircle3(list[support.x4_[0]], list[support.x4_[1]], list[idx]);
    support.x0_ = 3;
    support.x4_[2] = idx;
  }
  return ret;
}

static Circle2 UpdateSupport3(int idx, const zeus::CVector2f** list, Support& support) {
  Circle2 circs[6] = {};
  float intersect;
  int circIdxA = -1;
  int circIdxB = -1;
  float minRadA = FLT_MAX;
  float minRadB = FLT_MAX;

  circs[0] = ExactCircle2(list[support.x4_[0]], list[idx]);
  if (PointInsideCircle(*list[support.x4_[1]], circs[0], intersect)) {
    if (PointInsideCircle(*list[support.x4_[2]], circs[0], intersect)) {
      minRadA = circs[0].x8_radiusSq;
      circIdxA = 0;
    } else {
      minRadB = intersect;
      circIdxB = 0;
    }
  } else {
    minRadB = intersect;
    circIdxB = 0;
  }

  circs[1] = ExactCircle2(list[support.x4_[1]], list[idx]);
  if (circs[1].x8_radiusSq < minRadA) {
    if (PointInsideCircle(*list[support.x4_[0]], circs[1], intersect)) {
      if (PointInsideCircle(*list[support.x4_[2]], circs[1], intersect)) {
        minRadA = circs[1].x8_radiusSq;
        circIdxA = 1;
      } else if (intersect < minRadB) {
        minRadB = intersect;
        circIdxB = 1;
      }
    } else if (intersect < minRadB) {
      minRadB = intersect;
      circIdxB = 1;
    }
  }

  circs[2] = ExactCircle2(list[support.x4_[2]], list[idx]);
  if (circs[2].x8_radiusSq < minRadA) {
    if (PointInsideCircle(*list[support.x4_[0]], circs[2], intersect)) {
      if (PointInsideCircle(*list[support.x4_[1]], circs[2], intersect)) {
        minRadA = circs[2].x8_radiusSq;
        circIdxA = 2;
      } else if (intersect < minRadB) {
        minRadB = intersect;
        circIdxB = 2;
      }
    } else if (intersect < minRadB) {
      minRadB = intersect;
      circIdxB = 2;
    }
  }

  circs[3] = ExactCircle3(list[support.x4_[0]], list[support.x4_[1]], list[idx]);
  if (circs[3].x8_radiusSq < minRadA) {
    if (PointInsideCircle(*list[support.x4_[2]], circs[3], intersect)) {
      minRadA = circs[3].x8_radiusSq;
      circIdxA = 3;
    } else if (intersect < minRadB) {
      minRadB = intersect;
      circIdxB = 3;
    }
  }

  circs[4] = ExactCircle3(list[support.x4_[0]], list[support.x4_[2]], list[idx]);
  if (circs[4].x8_radiusSq < minRadA) {
    if (PointInsideCircle(*list[support.x4_[1]], circs[4], intersect)) {
      minRadA = circs[4].x8_radiusSq;
      circIdxA = 4;
    } else if (intersect < minRadB) {
      minRadB = intersect;
      circIdxB = 4;
    }
  }

  circs[5] = ExactCircle3(list[support.x4_[1]], list[support.x4_[2]], list[idx]);
  if (circs[5].x8_radiusSq < minRadA) {
    if (PointInsideCircle(*list[support.x4_[0]], circs[5], intersect)) {
      circIdxA = 5;
    } else if (intersect < minRadB) {
      circIdxB = 5;
    }
  }

  if (circIdxA == -1)
    circIdxA = circIdxB;

  switch (circIdxA) {
  case 0:
    support.x0_ = 2;
    support.x4_[1] = idx;
    break;
  case 1:
    support.x0_ = 2;
    support.x4_[0] = idx;
    break;
  case 2:
    support.x0_ = 2;
    support.x4_[0] = support.x4_[2];
    support.x4_[1] = idx;
    break;
  case 3:
    support.x4_[2] = idx;
    break;
  case 4:
    support.x4_[1] = idx;
    break;
  case 5:
    support.x4_[0] = idx;
    break;
  default:
    break;
  }

  return circs[circIdxA];
}

typedef Circle2 (*FSupport)(int idx, const zeus::CVector2f** list, Support& support);
static const FSupport SupportFuncs[] = {nullptr, UpdateSupport1, UpdateSupport2, UpdateSupport3};

static Circle MinCircle(const std::vector<zeus::CVector2f>& coords) {
  Circle2 ret = {};
  if (coords.size() >= 1) {
    std::unique_ptr<const zeus::CVector2f*[]> randArr(new const zeus::CVector2f*[coords.size()]);
    for (size_t i = 0; i < coords.size(); ++i)
      randArr[i] = &coords[i];
    for (int i = coords.size() - 1; i >= 0; --i) {
      int shuf = rand() % (i + 1);
      if (shuf != i)
        std::swap(randArr[i], randArr[shuf]);
    }
    ret = ExactCircle1(randArr[0]);

    Support support = {};
    support.x0_ = 1;
    for (size_t i = 1; i < coords.size();) {
      bool broke = false;
      for (int j = 0; j < support.x0_; ++j) {
        if ((*randArr[i] - *randArr[support.x4_[j]]).magSquared() < 0.01f) {
          broke = true;
          break;
        }
      }
      float intersect;
      if (!broke && !PointInsideCircle(*randArr[i], ret, intersect)) {
        Circle2 circ = SupportFuncs[support.x0_](i, randArr.get(), support);
        if (circ.x8_radiusSq > ret.x8_radiusSq) {
          i = 0;
          ret = circ;
          continue;
        }
      }
      ++i;
    }
  }
  return ret;
}

void CMapWorld::RecalculateWorldSphere(const CMapWorldInfo& mwInfo, const IWorld& wld) const {
  std::vector<zeus::CVector2f> coords;
  coords.reserve(x0_areas.size() * 8);
  float zMin = FLT_MAX;
  float zMax = -FLT_MAX;
  for (size_t i = 0; i < x0_areas.size(); ++i) {
    if (IsMapAreaValid(wld, i, true)) {
      const CMapArea* mapa = GetMapArea(i);
      if (mapa->GetIsVisibleToAutoMapper(mwInfo.IsWorldVisible(i), mwInfo.IsAreaVisible(i))) {
        zeus::CAABox aabb = mapa->GetBoundingBox().getTransformedAABox(mapa->GetAreaPostTransform(wld, i));
        for (int j = 0; j < 8; ++j) {
          const zeus::CVector3f point = aabb.getPoint(j);
          coords.push_back(point.toVec2f());
          zMin = std::min(point.z(), zMin);
          zMax = std::max(point.z(), zMax);
        }
      }
    }
  }

  Circle circle = MinCircle(coords);
  const_cast<CMapWorld*>(this)->x3c_worldSphereRadius = circle.x8_radius;
  const_cast<CMapWorld*>(this)->x30_worldSpherePoint =
      zeus::CVector3f(circle.x0_point.x(), circle.x0_point.y(), (zMin + zMax) * 0.5f);
  const_cast<CMapWorld*>(this)->x40_worldSphereHalfDepth = (zMax - zMin) * 0.5f;
}

zeus::CVector3f CMapWorld::ConstrainToWorldVolume(const zeus::CVector3f& point, const zeus::CVector3f& lookVec) const {
  zeus::CVector3f ret = point;
  if (std::fabs(lookVec.z()) > FLT_EPSILON) {
    float f2 = point.z() - (x40_worldSphereHalfDepth + x30_worldSpherePoint.z());
    float f1 = point.z() - (x30_worldSpherePoint.z() - x40_worldSphereHalfDepth);
    if (f2 > 0.f)
      ret = point + lookVec * (-f2 / lookVec.z());
    else if (f1 < 0.f)
      ret = point + lookVec * (-f1 / lookVec.z());
  } else {
    ret.z() = zeus::clamp(x30_worldSpherePoint.z() - x40_worldSphereHalfDepth, float(ret.z()),
                          x40_worldSphereHalfDepth + x30_worldSpherePoint.z());
  }

  zeus::CVector2f tmp = x30_worldSpherePoint.toVec2f();
  zeus::CVector2f vec2 = point.toVec2f() - tmp;
  if (vec2.magnitude() > x3c_worldSphereRadius) {
    tmp += vec2.normalized() * x3c_worldSphereRadius;
    ret.x() = float(tmp.x());
    ret.y() = float(tmp.y());
  }

  return ret;
}

void CMapWorld::ClearTraversedFlags() const {
  std::vector<bool>& flags = const_cast<CMapWorld*>(this)->x20_traversed;
  for (size_t i = 0; i < flags.size(); ++i)
    flags[i] = false;
}

CFactoryFnReturn FMapWorldFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                  CObjectReference* selfRef) {
  return TToken<CMapWorld>::GetIObjObjectFor(std::make_unique<CMapWorld>(in));
}

} // namespace urde
