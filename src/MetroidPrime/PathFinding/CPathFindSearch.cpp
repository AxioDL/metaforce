#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"

#include "MetroidPrime/PathFinding/CPathFindArea.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

CPFOpenList::CPFOpenList() {
  mRegion.SetData(&mRegionData);
  Clear();
}

CPathFindSearch::CPathFindSearch(CPFArea* area, uint flags, uint index, float chRadius,
                                 float chHeight)
: mArea(area)
, mCurWaypoint(0)
, mChHeight(chHeight)
, mChRadius(chRadius)
, mPadding(10.f)
, mFlags(flags)
, mIndexMask(1 << index) {}

CPathFindSearch::EResult CPathFindSearch::Search(const CVector3f& source,
                                                 const CVector3f& destination) {
  int i;
  int j;
  bool outsideSource = false;
  bool includeDest;
  bool outsideDest = false;
  mWaypoints.clear();
  mCurWaypoint = 0;
  if (!mArea) {
    mResult = kR_InvalidArea;
    return mResult;
  }
  if (mArea->GetNumRegions() > 512) {
    mResult = kR_InvalidArea;
    return mResult;
  }
  if (close_enough(source, destination)) {
    mWaypoints.push_back(source);
    mResult = kR_Success;
    return mResult;
  }

  CVector3f localSource = mArea->GetTransform().TransposeMultiply(source);
  CVector3f localDest = mArea->GetTransform().TransposeMultiply(destination);
  if (!(mFlags & 2) && !(mFlags & 4)) {
    localSource[kDZ] += 0.3f;
    localDest[kDZ] += 0.3f;
  }

  rstl::reserved_vector< CVector3f, 16 > points;
  rstl::reserved_vector< CPFRegion*, 4 > sourceRegions;
  if (mArea->FindRegions(sourceRegions, localSource, mFlags, mIndexMask) == 0) {
    CPFRegion* region =
        mArea->FindClosestRegion(localSource, mFlags, mIndexMask, mPadding);
    if (!region) {
      mResult = kR_NoSourcePoint;
      return mResult;
    }
    if (mFlags & 2 || mFlags & 4) {
      outsideSource = true;
      points.push_back(localSource);
    }
    sourceRegions.push_back(region);
    localSource = mArea->GetClosestPoint();
  }

  CVector3f finalDest = localDest;
  rstl::reserved_vector< CPFRegion*, 4 > destRegions;
  if (mArea->FindRegions(destRegions, localDest, mFlags, mIndexMask) == 0) {
    CPFRegion* region =
        mArea->FindClosestRegion(localDest, mFlags, mIndexMask, mPadding);
    if (!region) {
      mResult = kR_NoDestPoint;
      return mResult;
    }
    if (mFlags & 2 || mFlags & 4) {
      outsideDest = true;
    }
    destRegions.push_back(region);
    localDest = mArea->GetClosestPoint();
  }

  rstl::reserved_vector< CPFRegion*, 4 > uniqueSources;
  rstl::reserved_vector< CPFRegion*, 4 > uniqueDests;
  bool noPath = true;
  for (i = 0; i < sourceRegions.size(); ++i) {
    for (j = 0; j < destRegions.size(); ++j) {
      if (sourceRegions[i] == destRegions[j]) {
        if (!(mFlags & 2) && !(mFlags & 4)) {
          destRegions[j]->DropToGround(localSource);
          destRegions[j]->DropToGround(localDest);
        }
        const CTransform4f& transform = mArea->GetTransform();
        mWaypoints.push_back(transform * localSource);
        if (!close_enough(localSource, localDest)) {
          mWaypoints.push_back(transform * localDest);
        }
        if (outsideDest && !close_enough(localDest, finalDest)) {
          mWaypoints.push_back(transform * finalDest);
        }
        mResult = kR_Success;
        return mResult;
      }
      if (mArea->PathExists(sourceRegions[i], destRegions[j], mFlags)) {
        int sourceIdx = uniqueSources.size();
        while (--sourceIdx >= 0) {
          if (uniqueSources[sourceIdx] == sourceRegions[i]) {
            break;
          }
        }
        if (sourceIdx < 0) {
          uniqueSources.push_back(sourceRegions[i]);
        }
        int destIdx = uniqueDests.size();
        while (--destIdx >= 0) {
          if (uniqueDests[destIdx] == destRegions[j]) {
            break;
          }
        }
        if (destIdx < 0) {
          uniqueDests.push_back(destRegions[j]);
        }
        noPath = false;
      }
    }
  }

  if (noPath || !Search(uniqueSources, localSource, uniqueDests, localDest)) {
    mResult = kR_NoPath;
    return mResult;
  }

  CPFRegion* destRegion = uniqueDests[0];
  CPFRegion* sourceRegion = uniqueSources[0];
  CPFRegion* region = destRegion;
  int numLinks = 0;
  do {
    CPFRegion* parent = region->Data()->GetParent();
    parent->SetLinkTo(region->GetIndex());
    region = parent;
    ++numLinks;
  } while (region != sourceRegion);

  includeDest = true;
  int firstPoint = (outsideSource ? 1 : 0) + 1;
  int lastPoint = numLinks - 1;
  lastPoint += firstPoint;
  if (lastPoint >= points.capacity()) {
    lastPoint = points.capacity() - 1;
  }
  int outsidePoints = outsideDest ? 1 : 0;
  int pointCount = lastPoint;
  pointCount += outsidePoints;
  if (pointCount + 1 >= points.capacity()) {
    includeDest = false;
  }
  if (!(mFlags & 2) && !(mFlags & 4)) {
    sourceRegion->DropToGround(localSource);
    destRegion->DropToGround(localDest);
  }

  float halfHeight = 0.5f * mChHeight;
  points.push_back(localSource);
  region = sourceRegion;
  for (j = firstPoint; j <= lastPoint; ++j) {
    const CPFLink* link = region->GetPathLink();
    CPFRegion* linkRegion = &mArea->GetRegion(link->GetRegion());
    CVector3f midpoint = region->GetLinkMidPoint(*link);
    if (mFlags & 2 || mFlags & 4) {
      float height = CMath::Min(region->GetHeight(), linkRegion->GetHeight());
      float lower = halfHeight + midpoint[kDZ];
      float upper = height + midpoint[kDZ] - halfHeight;
      midpoint[kDZ] = CMath::Clamp(lower, destination[kDZ], upper);
    }
    points.push_back(midpoint);
    region = linkRegion;
  }
  if (includeDest) {
    points.push_back(localDest);
    if (outsideDest) {
      points.push_back(finalDest);
    }
  }

  for (int i = 0; i < 2; ++i) {
    region = sourceRegion;
    for (j = firstPoint; j <= (includeDest ? lastPoint : lastPoint - 1); ++j) {
      const CPFLink* link = region->GetPathLink();
      CPFRegion* linkRegion = &mArea->GetRegion(link->GetRegion());
      if (mFlags & 2 || mFlags & 4) {
        float height = CMath::Min(region->GetHeight(), linkRegion->GetHeight());
        points[j] = region->FitThroughLink3d(points[j - 1], *link, height, points[j + 1],
                                             mChRadius, halfHeight);
      } else {
        points[j] = region->FitThroughLink2d(points[j - 1], *link, points[j + 1], mChRadius);
      }
      region = linkRegion;
    }
  }

  const CTransform4f& transform = mArea->GetTransform();
  for (int i = 0; i < points.size(); ++i) {
    if (i == points.size() - 1 || !close_enough(points[i], points[i + 1])) {
      mWaypoints.push_back(transform * points[i]);
    }
  }
  mResult = kR_Success;
  return mResult;
}

bool CPathFindSearch::Search(rstl::reserved_vector< CPFRegion*, 4 >& sourceRegions,
                             const CVector3f& source,
                             rstl::reserved_vector< CPFRegion*, 4 >& destRegions,
                             const CVector3f& destination) {
  CPFBitSet& closedSet = mArea->ClosedSet();
  CPFOpenList& openList = mArea->OpenList();
  closedSet.Clear();
  openList.Clear();

  rstl::reserved_vector< CVector3f, 4 > destCentroids;
  for (int i = 0; i < destRegions.size(); ++i) {
    destCentroids.push_back(destRegions[i]->GetCentroid());
    destRegions[i]->SetCentroid(destination);
  }
  rstl::reserved_vector< CVector3f, 4 > sourceCentroids;
  float h = (destination - source).Magnitude();
  for (int i = 0; i < sourceRegions.size(); ++i) {
    CPFRegion* region = sourceRegions[i];
    sourceCentroids.push_back(region->GetCentroid());
    region->SetCentroid(source);
    region->Data()->Setup(nullptr, 0.f, h);
    openList.Push(region);
  }

  int i;
  CPFRegion* region;
  while ((region = openList.Pop()) != nullptr) {
    for (i = 0; i < destRegions.size(); ++i) {
      if (destRegions[i] == region) {
        goto found;
      }
    }
    closedSet.Add(region->GetIndex());
    for (i = 0; i < region->GetNumLinks(); ++i) {
      CPFRegion* linkRegion = &mArea->GetRegion(region->GetLink(i)->GetRegion());
      if (linkRegion != region->Data()->GetParent() &&
          (linkRegion->GetFlags() & 0xff & mFlags) &&
          ((linkRegion->GetFlags() >> 16) & 0xff & mIndexMask)) {
        float distance =
            CMath::FastSqrtF((linkRegion->GetCentroid() - region->GetCentroid()).MagSquared());
        float parentG = region->Data()->GetG();
        float g = parentG + distance;
        if ((!closedSet.Test(linkRegion->GetIndex()) && !openList.Test(linkRegion)) ||
            !(linkRegion->Data()->GetG() <= g)) {
          if (openList.Test(linkRegion)) {
            openList.Pop(linkRegion);
            linkRegion->Data()->Setup(region, g);
          } else {
            closedSet.Rmv(linkRegion->GetIndex());
            float nextH = CMath::FastSqrtF((destination - linkRegion->GetCentroid()).MagSquared());
            linkRegion->Data()->Setup(region, g, nextH);
          }
          openList.Push(linkRegion);
        }
      }
    }
  }

found:
  for (int i = 0; i < sourceRegions.size(); ++i) {
    sourceRegions[i]->SetCentroid(sourceCentroids[i]);
  }
  for (int i = 0; i < destRegions.size(); ++i) {
    destRegions[i]->SetCentroid(destCentroids[i]);
  }
  if (region) {
    destRegions.clear();
    destRegions.push_back(region);
    while (region->Data()->GetParent()) {
      region = region->Data()->GetParent();
    }
    sourceRegions.clear();
    sourceRegions.push_back(region);
  }
  return region != nullptr;
}

CPathFindSearch::EResult CPathFindSearch::FindClosestReachablePoint(const CVector3f& source,
                                                                    CVector3f& destination) const {
  if (!mArea) {
    return kR_InvalidArea;
  }

  CVector3f localSource = mArea->GetTransform().TransposeMultiply(source);
  CVector3f localDest = mArea->GetTransform().TransposeMultiply(destination);
  if (!(mFlags & 2) && !(mFlags & 4)) {
    localSource[kDZ] += 0.3f;
    localDest[kDZ] += 0.3f;
  }

  rstl::reserved_vector< CPFRegion*, 4 > regions;
  if (mArea->FindRegions(regions, localSource, mFlags, mIndexMask) == 0) {
    CPFRegion* region =
        mArea->FindClosestRegion(localSource, mFlags, mIndexMask, mPadding);
    if (!region) {
      return kR_NoSourcePoint;
    }
    regions.push_back(region);
  }

  CVector3f closestPoint =
      mArea->FindClosestReachablePoint(regions, localDest, mFlags, mIndexMask);
  closestPoint[kDZ] += 0.3f;
  destination = mArea->GetTransform() * closestPoint;
  return kR_Success;
}

CPathFindSearch::EResult CPathFindSearch::PathExists(const CVector3f& source,
                                                     const CVector3f& destination) const {
  if (!mArea) {
    return kR_InvalidArea;
  }

  CVector3f localSource = mArea->GetTransform().TransposeMultiply(source);
  CVector3f localDest = mArea->GetTransform().TransposeMultiply(destination);
  if (!(mFlags & 2) && !(mFlags & 4)) {
    localSource[kDZ] += 0.3f;
    localDest[kDZ] += 0.3f;
  }

  rstl::reserved_vector< CPFRegion*, 4 > sourceRegions;
  if (mArea->FindRegions(sourceRegions, localSource, mFlags, mIndexMask) == 0) {
    return kR_NoSourcePoint;
  }
  rstl::reserved_vector< CPFRegion*, 4 > destRegions;
  if (mArea->FindRegions(destRegions, localDest, mFlags, mIndexMask) == 0) {
    return kR_NoDestPoint;
  }
  for (int i = 0; i < sourceRegions.size(); ++i) {
    for (int j = 0; j < destRegions.size(); ++j) {
      if (sourceRegions[i] == destRegions[j] ||
          mArea->PathExists(sourceRegions[i], destRegions[j], mFlags)) {
        return kR_Success;
      }
    }
  }
  return kR_NoPath;
}

CPathFindSearch::EResult CPathFindSearch::OnPath(const CVector3f& point) const {
  if (!mArea) {
    return kR_InvalidArea;
  }
  CVector3f localPoint = mArea->GetTransform().TransposeMultiply(point);
  if (!(mFlags & 2) && !(mFlags & 4)) {
    localPoint[kDZ] += 0.3f;
  }
  rstl::reserved_vector< CPFRegion*, 4 > regions;
  if (mArea->FindRegions(regions, localPoint, mFlags, mIndexMask) == 0) {
    return kR_NoSourcePoint;
  }
  return kR_Success;
}
