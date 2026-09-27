#include "rstl/list.hpp"

#include "Kyoto/Animation/CSkinnedModel.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

typedef rstl::pair< CVector3f, rstl::list< uint > > TPosToVertListPair;

CSkinnedModelWithAvgNormals::CSkinnedModelWithAvgNormals(const CSkinnedModel& skinnedModel)
: mSkinnedModel(skinnedModel), mAvgNormals(rs_new float[skinnedModel.GetNumPoints() * 12]) {
#if VERSION >= VERSION_GM8P_00
  const uint vertexCount = skinnedModel.GetNumPoints();
#else
  int vertexCount = skinnedModel.GetNumPoints();
#endif
  const CVector3f* modelPositions =
      reinterpret_cast< const CVector3f* >(skinnedModel.GetModel()->GetPositions());

  rstl::vector< TPosToVertListPair > vertMap;
  vertMap.reserve(vertexCount);

  for (uint vertIdx = 0; vertIdx < vertexCount; ++vertIdx) {
    bool foundEqPos = false;
    uint vopolSize = vertMap.size();
    for (uint i = 0; i < vopolSize; ++i) {
      if (vertMap[i].first.IsEqu(modelPositions[vertIdx])) {
        foundEqPos = true;
        break;
      }
    }

    if (!foundEqPos) {
      rstl::list< uint > tmpList;
      for (uint j = vertIdx; j < vertexCount; ++j) {
        if (modelPositions[j] == modelPositions[vertIdx]) {
          tmpList.push_back(j);
        }
      }
      vertMap.push_back(TPosToVertListPair(modelPositions[vertIdx], tmpList));
    }
  }

  const CVector3f* normals =
      reinterpret_cast< const CVector3f* >(skinnedModel.GetModel()->GetNormals());
#if VERSION >= VERSION_GM8P_00
  CVector3f* avgNormals = reinterpret_cast< CVector3f* >(mAvgNormals.get());
  AUTO(mapCur, vertMap.begin());
  AUTO(mapEnd, vertMap.end());
#else
  float* avgNormals = mAvgNormals.get();
  TPosToVertListPair* mapCur = vertMap.mItems;
  TPosToVertListPair* mapEnd = mapCur + vertMap.mCount;
#endif
  for (; mapCur != mapEnd; ++mapCur) {
    CVector3f accum(0.f, 0.f, 0.f);

    AUTO(lit, mapCur->second.begin());
    AUTO(listEnd, mapCur->second.end());
    for (; lit != listEnd; ++lit) {
      accum += normals[*lit];
    }

    lit = mapCur->second.begin();
    CVector3f normalized = accum.AsNormalized();
    for (; lit != listEnd; ++lit) {
      reinterpret_cast< CVector3f* >(avgNormals)[*lit] = normalized;
    }
  }
}
