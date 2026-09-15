#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CCubeModel.hpp"
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "dolphin/gx/GXGeometry.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/types.h"

#include "Kyoto/Graphics/CTexture.hpp"
#include <Kyoto/Graphics/CModel.hpp>

#include <Kyoto/Graphics/CGraphics.hpp>
#include <Kyoto/IObjectStore.hpp>
#include <Kyoto/TToken.hpp>
static bool sIsTextureTimeoutEnabled = true;
uint CModel::sTotalMemory = 0;
CModel* CModel::sThisFrameList = nullptr;
CModel* CModel::sOneFrameList = nullptr;
CModel* CModel::sTwoFrameList = nullptr;
static uint sFrameCounter = 0;

static uchar* MemoryFromPartData(uchar*& dataCur, int*& secSizeCur) {
  uchar* ret = *secSizeCur != 0 ? dataCur : nullptr;
  dataCur += *secSizeCur;
  secSizeCur++;
  return ret;
}

CModel::CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store)
: x0_data(data.release())
, x4_dataLen(length)
, x28_modelInstance(nullptr)
, x2c_currentMatxIdx(0)
, x2e_lastMaterialFrame(0)
, x30_prev(nullptr)
, x34_next(sThisFrameList)
, x38_lastFrame(CGraphics::GetFrameCounter() - 2) {

  uint sectionSizeStart = 0x2c;
  uchar* dataPtr = data.get();
  const uint flags = *reinterpret_cast< const uint* >(dataPtr + 8);
  const uint version = *reinterpret_cast< const uint* >(dataPtr + 4);
  const uint visorFlags = (flags >> 1) & 1;
  const bool hasShortUvs = (flags >> 2) & 1;
  if (version == 1) {
    sectionSizeStart = 0x28;
  }

  const int sectionCount = *reinterpret_cast< const int* >(dataPtr + 0x24);
  uchar* dataCur;
  int* sectionSizes = reinterpret_cast< int* >(dataPtr + sectionSizeStart);
  int numMatSets = 1;
  if (version >= 2) {
    numMatSets = *reinterpret_cast< const int* >(dataPtr + 0x28);
  }
  int* secSizeCur = sectionSizes;
  dataCur = dataPtr + ((sectionSizeStart + sectionCount * 4 + 31) & ~31);
  x18_matSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; ++i) {
    x18_matSets.push_back(SShader(MemoryFromPartData(dataCur, secSizeCur)));
    SShader& shader = x18_matSets.back();
    CCubeModel::MakeTexturesFromMats(shader.x10_data, shader.x0_textures, store, true);
    x4_dataLen += shader.x0_textures.size() * 12;
  }

  const void* positions = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  const void* normals = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  const void* vtxColors = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  uint surfaceCount;
  const void* floatUvs = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  const void* shortUvs = nullptr;
  if (hasShortUvs) {
    shortUvs = reinterpret_cast< const void* >(MemoryFromPartData(dataCur, secSizeCur));
  }

  uint* surfaceInfo = reinterpret_cast< uint* >(MemoryFromPartData(dataCur, secSizeCur));
  surfaceCount = CBasics::SwapBytes(*surfaceInfo);
  x8_surfaces.reserve(surfaceCount);

  for (uint i = 0; i < surfaceCount; ++i) {
    x8_surfaces.push_back(MemoryFromPartData(dataCur, secSizeCur));
  }

  x28_modelInstance = rs_new CCubeModel(
      &x8_surfaces, &x18_matSets.front().x0_textures, x18_matSets.front().x10_data, positions,
      normals, vtxColors, floatUvs, shortUvs, *reinterpret_cast< const CAABox* >(dataPtr + 0xc),
      visorFlags ? 1 : 0, true, -1);
  sThisFrameList = this;
  if (x34_next != nullptr) {
    x34_next->x30_prev = this;
  }
  x4_dataLen += x8_surfaces.size() * 4;
  sTotalMemory += x4_dataLen;
  DCFlushRange(x0_data.get(), length);
}

CModel::~CModel() {
  RemoveFromList();
  RemoveFromTotal(x4_dataLen);
  const int frame = CGraphics::GetFrameCounter();
  if (x38_lastFrame == frame) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          x0_data.release());
  } else if (x38_lastFrame == frame - 1) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_ThisFrame,
                                          x0_data.release());
  }
}

void CModel::Draw(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    x28_modelInstance->DrawNormal(nullptr, nullptr, kSS_All);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  x28_modelInstance->Draw(flags);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    x28_modelInstance->DrawNormal(nullptr, nullptr, kSS_Unsorted);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  x28_modelInstance->DrawNormal(flags);
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    x28_modelInstance->DrawNormal(nullptr, nullptr, kSS_Sorted);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  x28_modelInstance->DrawAlpha(flags);
}

void CModel::Draw(const float* positions, const float* normals, const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    x28_modelInstance->DrawNormal(positions, normals, kSS_All);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  x28_modelInstance->Draw(positions, normals, flags);
}

void CModel::VerifyCurrentShader(int shader) const {
  if (shader >= x18_matSets.size()) {
    shader = 0;
  }
  if (shader == x2c_currentMatxIdx) {
    if (x2e_lastMaterialFrame != 0 && x2e_lastMaterialFrame <= sFrameCounter) {
      for (int i = 0; i < x18_matSets.size(); ++i) {
        if (i != shader) {
          x18_matSets[i].UnlockTextures();
        }
      }
      x2e_lastMaterialFrame = 0;
    }
  } else {
    x2c_currentMatxIdx = shader;
    SShader& material = x18_matSets[shader];
    x28_modelInstance->RemapMaterialData(material.x10_data, &material.x0_textures);
    if (x18_matSets.size() > 1) {
      x2e_lastMaterialFrame = sFrameCounter + 2;
    }
  }
}

const CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.x0_obj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  GXInvalidateVtxCache();
  return rs_new CModel(ptr, len, *pool);
}

const float* CModel::GetPositions() const {
  return static_cast< const float* >(x28_modelInstance->GetPositions());
}

const float* CModel::GetNormals() const {
  return static_cast< const float* >(x28_modelInstance->GetNormals());
}

void CModel::Touch(int shader) const {
  MoveToThisFrameList();
  VerifyCurrentShader(shader);
  if (x28_modelInstance->TryLockTextures()) {
    const rstl::vector< TCachedToken< CTexture > >& textures = x28_modelInstance->GetTextures();
    for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
      if (it->GetObject()) {
        (void)it->GetObject()->LoadToMRAM();
      }
    }
  }
}

bool CModel::IsLoaded(int shader) const {
  VerifyCurrentShader(shader);

  const rstl::vector< TCachedToken< CTexture > >& textures = x28_modelInstance->GetTextures();
  for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
    if (!it->IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CModel::MoveToThisFrameList() const {
  const_cast< CModel* >(this)->UpdateLastFrame();

  if (sThisFrameList == this) {
    return;
  }
  RemoveFromList();

  if (sThisFrameList != nullptr) {
    x34_next = sThisFrameList;
    x34_next->x30_prev = const_cast< CModel* >(this);
  }

  sThisFrameList = const_cast< CModel* >(this);
}

void CModel::RemoveFromList() const {
  if (x30_prev != nullptr) {
    x30_prev->x34_next = x34_next;
  } else if (this == sThisFrameList) {
    sThisFrameList = x34_next;
  } else if (this == sOneFrameList) {
    sOneFrameList = x34_next;
  } else if (this == sTwoFrameList) {
    sTwoFrameList = x34_next;
  }

  if (x34_next != nullptr) {
    x34_next->x30_prev = x30_prev;
  }

  x30_prev = nullptr;
  x34_next = nullptr;
}

void CModel::SShader::UnlockTextures() {
  for (AUTO(it, x0_textures.begin()); it != x0_textures.end(); ++it) {
    it->Unlock();
  }
}

void CModel::FrameDone() {
  ++sFrameCounter;
  if (!sIsTextureTimeoutEnabled) {
    return;
  }

  for (CModel* model = sTwoFrameList; model != nullptr;) {
    CModel* next = model->x34_next;
    model->VerifyCurrentShader(0);
    for (AUTO(it, model->x18_matSets.begin() + 1); it != model->x18_matSets.end(); ++it) {
      it->UnlockTextures();
    }
    model->x28_modelInstance->UnlockTextures();
    model->x30_prev = model->x34_next = nullptr;
    model = next;
  }

  sTwoFrameList = sOneFrameList;
  sOneFrameList = sThisFrameList;
  sThisFrameList = nullptr;
}

void CModel::DisableTextureTimeout() { sIsTextureTimeoutEnabled = false; }

void CModel::EnableTextureTimeout() { sIsTextureTimeoutEnabled = true; }

uint CModel::GetDataSize() const { return x4_dataLen; }

rstl::auto_ptr< uchar > CModel::GetData() { return rstl::auto_ptr< uchar >(x0_data.get()); }

namespace {
#ifdef __MWERKS__
inline void RemapPointer(const void*& pointer, uintptr_t offset) {
  if (pointer != nullptr) {
    pointer = reinterpret_cast< void* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
}
#endif

template < typename T >
inline void RemapPointer(T*& pointer, uintptr_t offset) {
#ifdef __MWERKS__
  RemapPointer(reinterpret_cast< const void*& >(pointer), offset);
#else
  if (pointer != nullptr) {
    pointer = reinterpret_cast< T* >(reinterpret_cast< uintptr_t >(pointer) + offset);
  }
#endif
}
} // namespace

void CModel::RemapData(uchar* data) {
  uintptr_t offset =
      reinterpret_cast< uintptr_t >(data) - reinterpret_cast< uintptr_t >(x0_data.release());
  x0_data = data;
  for (int i = 0; i < x18_matSets.size(); ++i) {
    RemapPointer(x18_matSets[i].x10_data, offset);
  }

  const CCubeModel::ModelInstance& instance = x28_modelInstance->GetModelInstance();
  const uchar* positions = static_cast< const uchar* >(instance.GetVertexPointer());
  const uchar* normals = static_cast< const uchar* >(instance.GetNormalPointer());
  const uchar* colors = static_cast< const uchar* >(instance.GetColorPointer());
  const uchar* uvs = static_cast< const uchar* >(instance.GetTCPointer());
  const uchar* packedUvs = static_cast< const uchar* >(instance.GetPackedTCPointer());
  const CAABox bounds = x28_modelInstance->GetBoundingBox();
  uchar flags = x28_modelInstance->GetModelFlags();
  bool texturesLoaded = x28_modelInstance->AreTexturesLoaded();
  const int index = x28_modelInstance->GetModelIndex();
  RemapPointer(positions, offset);
  RemapPointer(normals, offset);
  RemapPointer(colors, offset);
  RemapPointer(uvs, offset);
  RemapPointer(packedUvs, offset);
  for (int i = 0; i < x8_surfaces.size(); ++i) {
    RemapPointer(x8_surfaces[i], offset);
  }

  x28_modelInstance = rs_new CCubeModel(&x8_surfaces, &x18_matSets.front().x0_textures,
                                        x18_matSets.front().x10_data, positions, normals, colors,
                                        uvs, packedUvs, bounds, flags, texturesLoaded, index);
  MoveToThisFrameList();
}

void CModel::UpdateLastFrame() const { x38_lastFrame = CGraphics::GetFrameCounter(); }
