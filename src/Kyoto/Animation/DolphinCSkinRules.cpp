#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CPoseAsTransforms.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "dolphin/os/OSCache.h"
#include "rstl/math.hpp"

static int StreamFloatToShort(CInputStream& in) {
  const int result = in.Get< int >();
  if (result == -1) {
    return in.ReadLong();
  }
  uchar junk[780];
  for (int i = 0, iVar2 = 0; i < (result * 3); i += iVar2) {
    iVar2 = rstl::min_val(((result * 3) - i), 192);
    in.Get(junk, iVar2 * 4);
  }
  return result;
}

CSkinRules::CSkinRules(CInputStream& in)
: x0_virtualBones(in)
, x10_vertexCount(StreamFloatToShort(in))
, x14_normalCount(StreamFloatToShort(in)) {

  CModel::AddToTotal(x0_virtualBones.size() * sizeof(CVirtualBone) + sizeof(CSkinRules));
}

CSkinRules::~CSkinRules() {
  CModel::RemoveFromTotal(x0_virtualBones.size() * sizeof(CVirtualBone) + sizeof(CSkinRules));
}

void CSkinRules::BuildAccumulatedTransforms(const CPoseAsTransforms& pose,
                                            const CCharLayoutInfo& layoutInfo) const {
  float pointStorage[100][3];
  CVector3f* points = reinterpret_cast< CVector3f* >(pointStorage);
  CSegId id = pose.GetTransforms().GetFirstElementPresent();
  while (id != CSegId::Null()) {
    const CVector3f& origin = layoutInfo.GetReferenceStanceOffset(id);
    const CVector3f& rotatedOrigin = pose.GetTransformMinusOffset(id) * origin;
    points[id.val()] = pose.GetOffset(id) - rotatedOrigin;
    id = pose.GetTransforms().GetIdAfter(id);
  }

  for (int i = 0; i < x0_virtualBones.size(); ++i) {
    x0_virtualBones[i].BuildAccumulatedTransform(pose, points);
  }
}

void CSkinRules::BuildPoints(volatile void* pipe) const {
  for (int i = 0; i < x0_virtualBones.size(); ++i) {
    int vertexCount = x0_virtualBones[i].GetNumIndices();
    ushort* buffer = nullptr;
    for (int done = 0; done < vertexCount;) {
      const int count = ProcessingPoints(vertexCount - done, &buffer);
      x0_virtualBones[i].BuildPoints(buffer, pipe, count);
      done += count;
    }
  }
}

void CSkinRules::BuildNormals(volatile void* pipe) const {
  for (int i = 0; i < x0_virtualBones.size(); ++i) {
    int vertexCount = x0_virtualBones[i].GetNumIndices();
    ushort* buffer = nullptr;
    for (int done = 0; done < vertexCount;) {
      const int count = ProcessingNormals(vertexCount - done, &buffer);
      x0_virtualBones[i].BuildNormals(buffer, pipe, count);
      done += count;
    }
  }
}

void CSkinRules::BuildNormalsFrom(const CVector3f* averageNormals, CVector3f* out) const {
  int offset = 0;
  for (int i = 0; i < x0_virtualBones.size(); ++i) {
    const CVirtualBone& bone = x0_virtualBones[i];
    int count = bone.GetNumIndices();
    bone.BuildNormals(averageNormals + offset, out + offset, count);
    offset += count;
  }
}

const CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in,
                                         const CVParamTransfer&) {
  return rs_new CSkinRules(in);
}

static CSkinRules* sLockedRules = nullptr;
static const CModel* sCurrentTransaction = nullptr;
static int sCurrentPointCount = 0;
static bool sTransferringFirstPage = true;
static int sNextPointStart = 0;
static int sNextNormalStart = 0;
static int sCurrentPoint = 0;
static int sCurrentNormal = 0;
static CVector3f* sCurrentBase = nullptr;
static int sCurrentFirst = 0;
static int sTransactionCount = 0;

void CSkinRules::InitLockedCacheState(const CModel& model) {
  sLockedRules = this;
  sCurrentTransaction = &model;
  sTransferringFirstPage = true;
  sNextPointStart = 0;
  sNextNormalStart = 0;
  sCurrentPoint = 0;
  sCurrentNormal = 0;
  sCurrentBase = nullptr;
  sCurrentFirst = 0;
  sTransactionCount = 0;
  StartNextTransaction();
}

void CSkinRules::StartNextTransaction() {
  uchar* destination = reinterpret_cast< uchar* >(LCGetBase());
  if (!sTransferringFirstPage) {
    destination += 0x1000;
  }

  int count;
  const CVector3f* source;
  if (sNextPointStart != sLockedRules->GetNumPoints()) {
    count = rstl::min_val(336, sLockedRules->GetNumPoints() - sNextPointStart);
    source = static_cast< const CVector3f* >(sCurrentTransaction->GetCubeModel()->GetPositions()) +
             sNextPointStart;
  } else {
    const int normalCount = sLockedRules->GetNumNormals();
    if (normalCount == sNextNormalStart) {
      return;
    }
    count = rstl::min_val(336, normalCount - sNextNormalStart);
    source = static_cast< const CVector3f* >(sCurrentTransaction->GetCubeModel()->GetNormals()) +
             sNextNormalStart;
  }

  LCLoadData(destination, const_cast< CVector3f* >(source), (count * sizeof(CVector3f) + 31) & ~31);
  sCurrentPointCount = count;
  ++sTransactionCount;
  sTransferringFirstPage = !sTransferringFirstPage;
}

static void WaitForQueue() {
  if (!LCQueueLength()) {
    return;
  }
  LCQueueWait(0);
}

int CSkinRules::ProcessingPoints(int count, ushort** buf) {
  if (sCurrentPoint + count > sNextPointStart) {
    if (sCurrentPoint == sNextPointStart) {
      WaitForQueue();
      sCurrentFirst = sNextPointStart;
      sCurrentBase = reinterpret_cast< CVector3f* >(LCGetBase());
      if (sTransferringFirstPage) {
        sCurrentBase = reinterpret_cast< CVector3f* >(static_cast< uchar* >(LCGetBase()) + 0x1000);
      }

      sNextPointStart += sCurrentPointCount;
      StartNextTransaction();
    }

    int c = rstl::min_val(sNextPointStart - sCurrentPoint, count);
    *buf = reinterpret_cast< ushort* >(sCurrentBase + (sCurrentPoint - sCurrentFirst));
    sCurrentPoint += c;
    return c;
  } else {
    *buf = reinterpret_cast< ushort* >(sCurrentBase + (sCurrentPoint - sCurrentFirst));
    sCurrentPoint += count;
    return count;
  }
}

int CSkinRules::ProcessingNormals(int count, ushort** buf) {
  if (sCurrentNormal + count > sNextNormalStart) {
    if (sCurrentNormal == sNextNormalStart) {
      WaitForQueue();
      sCurrentFirst = sNextNormalStart;
      sCurrentBase = reinterpret_cast< CVector3f* >(LCGetBase());
      if (sTransferringFirstPage) {
        sCurrentBase = reinterpret_cast< CVector3f* >(static_cast< uchar* >(LCGetBase()) + 0x1000);
      }

      sNextNormalStart += sCurrentPointCount;
      StartNextTransaction();
    }

    int c = rstl::min_val(sNextNormalStart - sCurrentNormal, count);
    *buf = reinterpret_cast< ushort* >(sCurrentBase + (sCurrentNormal - sCurrentFirst));
    sCurrentNormal += c;
    return c;
  } else {
    *buf = reinterpret_cast< ushort* >(sCurrentBase + (sCurrentNormal - sCurrentFirst));
    sCurrentNormal += count;
    return count;
  }
}
