#include "Kyoto/Animation/CSkinnedModel.hpp"

#include "Kyoto/Alloc/CCircularBuffer.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"

#include <dolphin/PPCArch.h>
#include <dolphin/gx.h>
#include <dolphin/os.h>
#include <dolphin/os/OSCache.h>

#include <rstl/list.hpp>
#include <rstl/optional_object.hpp>

CSkinnedModel::TPointGenFunc CSkinnedModel::sPointGen;
void* CSkinnedModel::sPointGenData;

struct SSkinnedAllocation {
  SSkinnedAllocation(void* ptr, int w1, ushort w2) : x0_ptr(ptr), x4_unk1(w1), x8_unk2(w2) {}

  void* x0_ptr;
  int x4_unk1;
  ushort x8_unk2;
};

namespace Skinning {
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
static void* sStaticSkinningData = nullptr;
static int sStaticSkinningDataSize = 0x80000;
#endif
static ushort skCurrentToken = 0;
static int sNumSkinnedObjects = 0;
static bool sSkinningInitialized = false;
#if VERSION < VERSION_GM8P_00 || VERSION == VERSION_GM8E_02
static char sStaticSkinningData[0x80000] ATTRIBUTE_ALIGN(32);
#endif
static rstl::optional_object< CCircularBuffer > sSkinningBuffer;
static rstl::list< SSkinnedAllocation > sAllocations;
static bool sbDumpedSpinLockMessage = false;

void AddSkinnedRef();
void DelSkinnedRef();
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void SetSkinningBuffer(void* buffer, int size);
#endif
} // namespace Skinning

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
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
  if (!sSkinningInitialized) {
    GXSetDrawSync(0xFFFF);
    while (GXReadDrawSync() != 0xFFFF) {
    }
    skCurrentToken = 1;
    sSkinningInitialized = true;
  }

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  if (sNumSkinnedObjects++ == 0) {
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sStaticSkinningDataSize);
  }
#else
  if (sNumSkinnedObjects == 0) {
    sSkinningBuffer = CCircularBuffer(sStaticSkinningData, sizeof(sStaticSkinningData));
  }
  ++sNumSkinnedObjects;
#endif
}

void Skinning::DelSkinnedRef() {
  --sNumSkinnedObjects;
  if (sNumSkinnedObjects == 0) {
    sSkinningBuffer.clear();
    sAllocations.clear();
  }
}

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
void CSkinnedModel::SetSkinningBuffer(void* buffer, int size) {
  Skinning::SetSkinningBuffer(buffer, size);
}
#endif

CSkinnedModel::CSkinnedModel(const TLockedToken< CModel >& model,
                             const TLockedToken< CSkinRules >& skinRules,
                             const TLockedToken< CCharLayoutInfo >& layoutInfo,
                             EDataOwnership ownership)
: x4_model(model)
, x10_skinRules(skinRules)
, x1c_layoutInfo(layoutInfo)
, x28_vertWorkspace()
, x30_normalWorkspace()
, x38_owned(ownership == kDO_Owned)
, x39_disableWorkspaces(false) {
  Construct();
}

CSkinnedModel::CSkinnedModel(const CSkinnedModel& other)
: x4_model(other.x4_model)
, x10_skinRules(other.x10_skinRules)
, x1c_layoutInfo(other.x1c_layoutInfo)
, x28_vertWorkspace()
, x30_normalWorkspace()
, x38_owned(other.x38_owned)
, x39_disableWorkspaces(false) {
  Construct();
}

CSkinnedModel::~CSkinnedModel() { Skinning::DelSkinnedRef(); }

void CSkinnedModel::Construct() {
  Skinning::AddSkinnedRef();
  if (!x38_owned) {
    uint numPoints = x10_skinRules->GetNumPoints();
    uint numNormals = x10_skinRules->GetNumNormals();
    x28_vertWorkspace = rs_new float[numPoints * 12];
    x30_normalWorkspace = rs_new float[numNormals * 12];
  }
  if (x10_skinRules->GetNumVirtualBones() == 1) {
    x39_disableWorkspaces = true;
  }
}

void CSkinnedModel::Draw(const CModelFlags& flags) const {
  if (x39_disableWorkspaces) {
    CTransform4f saved(CGraphics::GetModelMatrix());
    CGraphics::SetModelMatrix(saved * x10_skinRules->GetVirtualBones()[0].GetTransform());
    x4_model->Draw(flags);
    CGraphics::SetModelMatrix(saved);
  } else if (x28_vertWorkspace.null()) {
    x4_model->Draw(flags);
  } else {
    x4_model->Draw(x28_vertWorkspace.get(), x30_normalWorkspace.get(), flags);
    PostDrawFunc();
  }
}

void CSkinnedModel::Draw(const TDrawFunc func, void* data) {
  if (x39_disableWorkspaces) {
    CTransform4f saved(CGraphics::GetModelMatrix());
    CGraphics::SetModelMatrix(saved * x10_skinRules->GetVirtualBones()[0].GetTransform());
    Draw(func, x4_model->GetPositions(), x4_model->GetNormals(), data);
    CGraphics::SetModelMatrix(saved);
  } else if (x28_vertWorkspace.null()) {
    Draw(func, x4_model->GetPositions(), x4_model->GetNormals(), data);
  } else {
    func(x28_vertWorkspace.get(), x30_normalWorkspace.get(), data);
    uint vertSize = (x10_skinRules->GetNumPoints() * 12 + 31) & ~31u;
    DCFlushRangeNoSync(x28_vertWorkspace.get(), vertSize);
    uint normSize = (x10_skinRules->GetNumNormals() * 12 + 31) & ~31u;
    DCFlushRangeNoSync(x30_normalWorkspace.get(), normSize);
    PPCSync();
    PostDrawFunc();
  }
}

void CSkinnedModel::Draw(const float* positions, const float* normals,
                         const CModelFlags& flags) const {
  x4_model->Draw(positions, normals, flags);
  PostDrawFunc();
}

void CSkinnedModel::Calculate(const CPoseAsTransforms& pose,
                              const rstl::optional_object< CVertexMorphEffect >& morphEffect,
                              const float* averagedNormals, float* workVerts) {
  size_t alignedNormSize = 0;
  size_t alignedVertSize = 0;
  size_t totalSize = 0;
  size_t vertSize = x10_skinRules->GetNumPoints() * sizeof(CVector3f);
  size_t normSize = x10_skinRules->GetNumNormals() * sizeof(CVector3f);
  float* verts;

  if (workVerts != nullptr) {
    verts = workVerts;
  } else {
    if (x39_disableWorkspaces) {
      x10_skinRules->BuildAccumulatedTransforms(pose, **x1c_layoutInfo);
      return;
    }
    AllocateStorage();
    verts = x28_vertWorkspace.get();
  }

  alignedNormSize = ((normSize + 31) & ~31u);
  alignedVertSize = ((vertSize + 31) & ~31u);
  totalSize = alignedVertSize + alignedNormSize;

  DCFlushRange(verts, totalSize);
  BOOL interruptState = OSDisableInterrupts();
  volatile void* pipe = GXRedirectWriteGatherPipe(verts);

  x10_skinRules->InitLockedCacheState(**x4_model);
  x10_skinRules->BuildAccumulatedTransforms(pose, **x1c_layoutInfo);
  x10_skinRules->BuildPoints(pipe);

  int numWords = x10_skinRules->GetNumPoints() * 3;
  int padWords = ((numWords + 7) & ~7) - numWords;
  for (int i = 0; i < padWords; i++) {
    *reinterpret_cast< volatile u32* >(pipe) = 0;
  }

  x10_skinRules->BuildNormals(pipe);
  GXRestoreWriteGatherPipe();
  OSRestoreInterrupts(interruptState);

  if (morphEffect.valid()) {
    (*morphEffect)
        .MorphVertices(reinterpret_cast< CVector3f* >(verts),
                       reinterpret_cast< const CVector3f* >(averagedNormals), x10_skinRules, pose,
                       x10_skinRules->GetNumPoints());
    DCFlushRange(verts, alignedVertSize);
  }

  if (sPointGen != nullptr) {
    const CVector3f* positions = reinterpret_cast< const CVector3f* >(verts);
    sPointGen(sPointGenData, positions, positions + x10_skinRules->GetNumPoints(),
              x10_skinRules->GetNumPoints());
    DCInvalidateRange(verts, totalSize);
  }
}

void CSkinnedModel::CalculateDefault() {
  x28_vertWorkspace = rstl::auto_ptr< float >();
  x30_normalWorkspace = rstl::auto_ptr< float >();
}

void CSkinnedModel::TickAllocations() {
  int syncVal = GXReadDrawSync();
  if (syncVal > static_cast< int >(Skinning::skCurrentToken)) {
    syncVal -= 0x10000;
  }
  while (Skinning::sAllocations.size() != 0) {
    SSkinnedAllocation& front = Skinning::sAllocations.front();
    int tokenVal = static_cast< int >(front.x8_unk2);
    if (tokenVal > static_cast< int >(Skinning::skCurrentToken)) {
      tokenVal -= 0x10000;
    }
    if (syncVal < tokenVal)
      break;
    Skinning::sSkinningBuffer->Free(front.x0_ptr, front.x4_unk1);
    Skinning::sAllocations.pop_front();
  }
}

void* CSkinnedModel::EnsureAllocation(int size) {
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
}

void CSkinnedModel::AllocateStorage() {
  if (x38_owned && (x28_vertWorkspace.null() || x30_normalWorkspace.null())) {
    int vertexCount = x10_skinRules->GetNumPoints();
    int normalCount = x10_skinRules->GetNumNormals();
    TickAllocations();
    int normSize = (normalCount * 12 + 31) & ~31;
    int vertSize = (vertexCount * 12 + 31) & ~31;
    int totalSize = vertSize + normSize + 32;
    void* ptr = EnsureAllocation(totalSize);
    if (ptr == Skinning::sStaticSkinningData) {
      GXInvalidateVtxCache();
    }
    x28_vertWorkspace = rstl::auto_ptr< float >(static_cast< float* >(ptr));
    x30_normalWorkspace =
        rstl::auto_ptr< float >(reinterpret_cast< float* >(static_cast< char* >(ptr) + vertSize));
    x28_vertWorkspace.release();
    x30_normalWorkspace.release();
  }
}

void CSkinnedModel::PostDrawFunc() const {
  if (x38_owned && !x28_vertWorkspace.null()) {
    x28_vertWorkspace = rstl::auto_ptr< float >();
    x30_normalWorkspace = rstl::auto_ptr< float >();
    GXSetDrawSync(Skinning::skCurrentToken);
    ++Skinning::skCurrentToken;
  }
}

void CSkinnedModel::AddDummySkinnedModelRef() { Skinning::AddSkinnedRef(); }

void CSkinnedModel::RemoveDummySkinnedModelRef() { Skinning::DelSkinnedRef(); }

void CSkinnedModel::SetPointGeneratorFunc(void* data, void (*func)(void*, const CVector3f*,
                                                                   const CVector3f*, int)) {
  sPointGen = func;
  sPointGenData = data;
}

void CSkinnedModel::ClearPointGeneratorFunc() { sPointGen = nullptr; }

float* CSkinnedModel::AllocateNewWorkspace(float** vertOut) {
  const CSkinRules* skinRules = *x10_skinRules;
  int normalCount = skinRules->GetNumNormals();
  int vertexCount = skinRules->GetNumPoints();
  int alignedNormSize = (normalCount * 12 + 31) & ~31;
  int alignedVertSize = (vertexCount * 12 + 31) & ~31;
  int vertSize = vertexCount * 12;
  float* ptr = static_cast< float* >(
      CMemory::Alloc(((vertSize + 31) & ~31) + alignedNormSize, IAllocator::kHI_RoundUpLen));
  if (vertOut != nullptr) {
    *vertOut = reinterpret_cast< float* >(reinterpret_cast< char* >(ptr) + alignedNormSize);
  }
  return ptr;
}
