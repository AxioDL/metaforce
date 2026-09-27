#include "Kyoto/Animation/CSkinnedModel.hpp"

#include "Kyoto/Alloc/CCircularBuffer.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"

#include <dolphin/base/PPCArch.h>
#include <dolphin/gx.h>
#include <dolphin/os.h>

#include <rstl/list.hpp>
#include <rstl/optional_object.hpp>

CSkinnedModel::TPointGenFunc CSkinnedModel::sPointGen;
void* CSkinnedModel::sPointGenData;

struct SSkinnedAllocation {
  SSkinnedAllocation(void* ptr, int w1, ushort w2) : mPtr(ptr), mUnk1(w1), mUnk2(w2) {}

  void* mPtr;
  int mUnk1;
  ushort mUnk2;
};

namespace Skinning {
#if VERSION >= VERSION_GM8P_00
static void* sStaticSkinningData = nullptr;
static int sStaticSkinningDataSize = 0x80000;
#endif
static ushort skCurrentToken = 0;
static int sNumSkinnedObjects = 0;
static bool sSkinningInitialized = false;
#if VERSION < VERSION_GM8P_00
ATTRIBUTE_ALIGN_DECL(32, static char sStaticSkinningData[0x80000]);
#endif
static rstl::optional_object< CCircularBuffer > sSkinningBuffer;
static rstl::list< SSkinnedAllocation > sAllocations;
static bool sbDumpedSpinLockMessage = false;

void AddSkinnedRef();
void DelSkinnedRef();
#if VERSION >= VERSION_GM8P_00
void SetSkinningBuffer(void* buffer, int size);
#endif
} // namespace Skinning

#if VERSION >= VERSION_GM8P_00
void Skinning::SetSkinningBuffer(void* buffer, int size) {
  sStaticSkinningDataSize = size;
  sAllocations.clear();
  sStaticSkinningData = buffer;
  if (buffer != nullptr && sNumSkinnedObjects != 0) {
    sSkinningBuffer = CCircularBuffer(buffer, sStaticSkinningDataSize);
  }
}
#endif

void Skinning::AddSkinnedRef() {
#if defined(TARGET_PC)
  ++sNumSkinnedObjects;
#else
  if (!sSkinningInitialized) {
    GXSetDrawSync(0xFFFF);
    while (GXReadDrawSync() != 0xFFFF) {
    }
    skCurrentToken = 1;
    sSkinningInitialized = true;
  }

#if VERSION >= VERSION_GM8P_00
  if (sNumSkinnedObjects++ == 0) {
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sStaticSkinningDataSize);
  }
#else
  if (sNumSkinnedObjects == 0) {
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sizeof(sStaticSkinningData));
  }
  ++sNumSkinnedObjects;
#endif
#endif
}

void Skinning::DelSkinnedRef() {
  --sNumSkinnedObjects;
#if !defined(TARGET_PC)
  if (sNumSkinnedObjects == 0) {
    sSkinningBuffer.clear();
    sAllocations.clear();
  }
#endif
}

#if VERSION >= VERSION_GM8P_00
void CSkinnedModel::SetSkinningBuffer(void* buffer, int size) {
  Skinning::SetSkinningBuffer(buffer, size);
}
#endif

CSkinnedModel::CSkinnedModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skinRules,
                             const TLockedToken< CCharLayoutInfo >& layoutInfo,
                             EDataOwnership ownership)
: mModel(model)
, mSkinRules(skinRules)
, mLayoutInfo(layoutInfo)
, mVertWorkspace()
, mNormalWorkspace()
, mOwned(ownership == kDO_Owned)
, mDisableWorkspaces(false) {
  Construct();
}

CSkinnedModel::CSkinnedModel(const CSkinnedModel& other)
: mModel(other.mModel)
, mSkinRules(other.mSkinRules)
, mLayoutInfo(other.mLayoutInfo)
, mVertWorkspace()
, mNormalWorkspace()
, mOwned(other.mOwned)
, mDisableWorkspaces(false) {
  Construct();
}

CSkinnedModel::~CSkinnedModel() { Skinning::DelSkinnedRef(); }

void CSkinnedModel::Construct() {
  Skinning::AddSkinnedRef();
  if (!mOwned) {
    uint numPoints = mSkinRules->GetNumPoints();
    uint numNormals = mSkinRules->GetNumNormals();
    mVertWorkspace = rs_new float[numPoints * 12];
    mNormalWorkspace = rs_new float[numNormals * 12];
  }
  if (mSkinRules->GetNumVirtualBones() == 1) {
    mDisableWorkspaces = true;
  }
}

void CSkinnedModel::Draw(const CModelFlags& flags) const {
  if (mDisableWorkspaces) {
    CTransform4f saved(CGraphics::GetModelMatrix());
    CGraphics::SetModelMatrix(saved * mSkinRules->GetVirtualBones()[0].GetTransform());
    mModel->Draw(flags);
    CGraphics::SetModelMatrix(saved);
  } else if (mVertWorkspace.null()) {
    mModel->Draw(flags);
  } else {
#if defined(TARGET_PC)
    mModel->Draw(GetPositionView(mVertWorkspace.get()),
                   GetNormalView(mNormalWorkspace.get()), flags);
#else
    mModel->Draw(mVertWorkspace.get(), mNormalWorkspace.get(), flags);
#endif
    PostDrawFunc();
  }
}

void CSkinnedModel::Draw(const TDrawFunc func, void* data) {
  if (mDisableWorkspaces) {
    CTransform4f saved(CGraphics::GetModelMatrix());
    CGraphics::SetModelMatrix(saved * mSkinRules->GetVirtualBones()[0].GetTransform());
#if defined(TARGET_PC)
    Draw(func, TModelPositions(), TModelNormals(), data);
#else
    Draw(func, mModel->GetPositions(), mModel->GetNormals(), data);
#endif
    CGraphics::SetModelMatrix(saved);
  } else if (mVertWorkspace.null()) {
#if defined(TARGET_PC)
    Draw(func, TModelPositions(), TModelNormals(), data);
#else
    Draw(func, mModel->GetPositions(), mModel->GetNormals(), data);
#endif
  } else {
#if defined(TARGET_PC)
    func(GetPositionView(mVertWorkspace.get()), GetNormalView(mNormalWorkspace.get()), data);
#else
    func(mVertWorkspace.get(), mNormalWorkspace.get(), data);
#endif
    uint vertSize = (mSkinRules->GetNumPoints() * 12 + 31) & ~31u;
    DCFlushRangeNoSync(mVertWorkspace.get(), vertSize);
    uint normSize = (mSkinRules->GetNumNormals() * 12 + 31) & ~31u;
    DCFlushRangeNoSync(mNormalWorkspace.get(), normSize);
    PPCSync();
    PostDrawFunc();
  }
}

void CSkinnedModel::Draw(const float* positions, const float* normals,
                         const CModelFlags& flags) const {
#if defined(TARGET_PC)
  mModel->Draw(GetPositionView(positions), GetNormalView(normals), flags);
#else
  mModel->Draw(positions, normals, flags);
#endif
  PostDrawFunc();
}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                              const float* averagedNormals, float* workVerts) {
  size_t alignedNormSize = 0;
  size_t alignedVertSize = 0;
  size_t totalSize = 0;
  size_t vertSize = mSkinRules->GetNumPoints() * sizeof(CVector3f);
  size_t normSize = mSkinRules->GetNumNormals() * sizeof(CVector3f);
  float* verts;

  if (workVerts != nullptr) {
    verts = workVerts;
  } else {
    if (mDisableWorkspaces) {
      mSkinRules->BuildAccumulatedTransforms(pose, **mLayoutInfo);
      return;
    }
    AllocateStorage();
    verts = mVertWorkspace.get();
  }

  alignedNormSize = ((normSize + 31) & ~31u);
  alignedVertSize = ((vertSize + 31) & ~31u);
  totalSize = alignedVertSize + alignedNormSize;

#if defined(TARGET_PC)
  mSkinRules->InitLockedCacheState(**mModel);
  mSkinRules->BuildAccumulatedTransforms(pose, **mLayoutInfo);
  mSkinRules->BuildPoints(verts);
  float* normals =
      workVerts != nullptr
          ? reinterpret_cast< float* >(reinterpret_cast< uchar* >(verts) + alignedVertSize)
          : mNormalWorkspace.get();
  mSkinRules->BuildNormals(normals);
#else
  DCFlushRange(verts, totalSize);
  BOOL interruptState = OSDisableInterrupts();
  volatile void* pipe = GXRedirectWriteGatherPipe(verts);

  mSkinRules->InitLockedCacheState(**mModel);
  mSkinRules->BuildAccumulatedTransforms(pose, **mLayoutInfo);
  mSkinRules->BuildPoints(pipe);

  int numWords = mSkinRules->GetNumPoints() * 3;
  int padWords = ((numWords + 7) & ~7) - numWords;
  for (int i = 0; i < padWords; i++) {
    *reinterpret_cast< volatile u32* >(pipe) = 0;
  }

  mSkinRules->BuildNormals(pipe);
  GXRestoreWriteGatherPipe();
  OSRestoreInterrupts(interruptState);
#endif

  if (morphEffect.valid()) {
    (*morphEffect)
        .MorphVertices(reinterpret_cast< CVector3f* >(verts),
                       reinterpret_cast< const CVector3f* >(averagedNormals), mSkinRules, pose,
                       mSkinRules->GetNumPoints());
    DCFlushRange(verts, alignedVertSize);
  }

  if (sPointGen != nullptr) {
    const CVector3f* positions = reinterpret_cast< const CVector3f* >(verts);
#if defined(TARGET_PC)
    sPointGen(sPointGenData, positions, reinterpret_cast< const CVector3f* >(normals),
              mSkinRules->GetNumPoints());
#else
    sPointGen(sPointGenData, positions, positions + mSkinRules->GetNumPoints(),
              mSkinRules->GetNumPoints());
#endif
    DCInvalidateRange(verts, totalSize);
  }
}

void CSkinnedModel::CalculateDefault() {
  mVertWorkspace = rstl::auto_ptr< float >();
  mNormalWorkspace = rstl::auto_ptr< float >();
}

void CSkinnedModel::TickAllocations() {
#if !defined(TARGET_PC)
  int syncVal = GXReadDrawSync();
  if (syncVal > static_cast< int >(Skinning::skCurrentToken)) {
    syncVal -= 0x10000;
  }
  while (Skinning::sAllocations.size() != 0) {
    SSkinnedAllocation& front = Skinning::sAllocations.front();
    int tokenVal = static_cast< int >(front.mUnk2);
    if (tokenVal > static_cast< int >(Skinning::skCurrentToken)) {
      tokenVal -= 0x10000;
    }
    if (syncVal < tokenVal)
      break;
    Skinning::sSkinningBuffer->Free(front.mPtr, front.mUnk1);
    Skinning::sAllocations.pop_front();
  }
#endif
}

void* CSkinnedModel::EnsureAllocation(int size) {
#if defined(TARGET_PC)
  return rs_new uchar[size];
#else
  size = (size + 31) & ~31;
  void* ptr = Skinning::sSkinningBuffer->Alloc(size);
  if (ptr == nullptr && !Skinning::sbDumpedSpinLockMessage) {
    Skinning::sSkinningBuffer->GetAllocatedAmount();
    Skinning::sbDumpedSpinLockMessage = true;
  }
  s32 startTick = OSGetTick();
  while (ptr == nullptr) {
    TickAllocations();
    ptr = Skinning::sSkinningBuffer->Alloc(size);
    if (ptr == nullptr) {
      s32 currentTick = OSGetTick();
      if (OSTicksToMilliseconds(static_cast< uint >(currentTick - startTick)) > 60) {
        GXReadDrawSync();
        for (AUTO(it, Skinning::sAllocations.begin()); it != Skinning::sAllocations.end(); ++it) {
        }
        startTick = currentTick;
        GXSetDrawSync(Skinning::skCurrentToken);
        ++Skinning::skCurrentToken;
      }
    }
  }
  Skinning::sAllocations.push_back(SSkinnedAllocation(ptr, size, Skinning::skCurrentToken));
  return ptr;
#endif
}

void CSkinnedModel::AllocateStorage() {
#if defined(TARGET_PC)
  if (mVertWorkspace.null()) {
    mVertWorkspace = rs_new float[mSkinRules->GetNumPoints() * 3];
  }
  if (mNormalWorkspace.null()) {
    mNormalWorkspace = rs_new float[mSkinRules->GetNumNormals() * 3];
  }
#else
  if (mOwned && (mVertWorkspace.null() || mNormalWorkspace.null())) {
    int vertexCount = mSkinRules->GetNumPoints();
    int normalCount = mSkinRules->GetNumNormals();
    TickAllocations();
    int normSize = (normalCount * 12 + 31) & ~31;
    int vertSize = (vertexCount * 12 + 31) & ~31;
    int totalSize = vertSize + normSize + 32;
    void* ptr = EnsureAllocation(totalSize);
    if (ptr == Skinning::sStaticSkinningData) {
      GXInvalidateVtxCache();
    }
    mVertWorkspace = rstl::auto_ptr< float >(static_cast< float* >(ptr));
    mNormalWorkspace =
        rstl::auto_ptr< float >(reinterpret_cast< float* >(static_cast< char* >(ptr) + vertSize));
    mVertWorkspace.release();
    mNormalWorkspace.release();
  }
#endif
}

void CSkinnedModel::PostDrawFunc() const {
#if defined(TARGET_PC)
  if (mOwned && !mVertWorkspace.null()) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mVertWorkspace.release());
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mNormalWorkspace.release());
    mVertWorkspace = rstl::auto_ptr< float >();
    mNormalWorkspace = rstl::auto_ptr< float >();
  }
#else
  if (mOwned && !mVertWorkspace.null()) {
    mVertWorkspace = rstl::auto_ptr< float >();
    mNormalWorkspace = rstl::auto_ptr< float >();
    GXSetDrawSync(Skinning::skCurrentToken);
    ++Skinning::skCurrentToken;
  }
#endif
}

void CSkinnedModel::AddDummySkinnedModelRef() { Skinning::AddSkinnedRef(); }

void CSkinnedModel::RemoveDummySkinnedModelRef() { Skinning::DelSkinnedRef(); }

void CSkinnedModel::SetPointGeneratorFunc(void* data, void (*func)(void*, const CVector3f*,
                                                                   const CVector3f*, int)) {
  sPointGen = func;
  sPointGenData = data;
}

void CSkinnedModel::ClearPointGeneratorFunc() { sPointGen = nullptr; }

float* CSkinnedModel::AllocateNewWorkspace(float** nrmOut) {
  const CSkinRules* skinRules = *mSkinRules;
  int normalCount = skinRules->GetNumNormals();
  int vertexCount = skinRules->GetNumPoints();
  int alignedNormSize = (normalCount * 12 + 31) & ~31;
  int alignedVertSize = (vertexCount * 12 + 31) & ~31;
  int vertSize = vertexCount * 12;
  float* ptr = static_cast< float* >(
      CMemory::Alloc(((vertSize + 31) & ~31) + alignedNormSize, IAllocator::kHI_RoundUpLen));
  if (nrmOut != nullptr) {
#if NONMATCHING
    // Wrong variable used originally
    *nrmOut = reinterpret_cast< float* >(reinterpret_cast< char* >(ptr) + alignedVertSize);
#else
    *nrmOut = reinterpret_cast< float* >(reinterpret_cast< char* >(ptr) + alignedNormSize);
#endif
  }
  return ptr;
}
