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

#if defined(TARGET_PC)
#include "Metaforce/CModelSectionReader.hpp"
#include <borealis/log.hpp>
#include <limits>

namespace {
constexpr borealis::Log Log{"CModel"};
} // namespace
#endif

static bool sIsTextureTimeoutEnabled = true;
uint CModel::sTotalMemory = 0;
CModel* CModel::sThisFrameList = nullptr;
CModel* CModel::sOneFrameList = nullptr;
CModel* CModel::sTwoFrameList = nullptr;
static uint sFrameCounter = 0;

#if !defined(TARGET_PC)
static uchar* MemoryFromPartData(uchar*& dataCur, int*& secSizeCur) {
  uchar* ret = *secSizeCur != 0 ? dataCur : nullptr;
  dataCur += *secSizeCur;
  secSizeCur++;
  return ret;
}
#endif

CModel::CModel(const rstl::auto_ptr< uchar >& data, int length, IObjectStore& store)
: mData(data.release())
, mDataLen(length)
, mModelInstance(nullptr)
, mCurrentMatxIdx(0)
, mLastMaterialFrame(0)
, mPrev(nullptr)
, mNext(sThisFrameList)
, mLastFrame(CGraphics::GetFrameCounter() - 2) {
#if defined(TARGET_PC)
  if (length < 0 || mData.null()) {
    Log.fatal("Invalid CMDL resource");
  }
  mResourceSize = length;
  CModelSectionReader reader({mData.get(), mResourceSize});
  mMatSets.reserve(reader.GetMaterialSetCount());
  for (uint i = 0; i < reader.GetMaterialSetCount(); ++i) {
    mMatSets.push_back(SShader(reader.Next()));
    auto& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
  }

  const auto positions = reader.Next();
  const auto normals = reader.Next();
  const auto colors = reader.Next();
  const auto texCoords = reader.Next();
  const auto packedTexCoords = (reader.GetFlags() & 4) ? reader.Next() : std::span< uchar >{};
  const auto surfaceInfo = reader.Next();
  const uint surfaceCount = ReadModelSurfaceCount(surfaceInfo, reader.GetRemainingSections());

  uint materialCount = std::numeric_limits< uint >::max();
  for (const auto& shader : mMatSets) {
    const uint count = ValidateModelMaterials(shader.mData);
    if (count < materialCount) {
      materialCount = count;
    }
  }

  std::vector< TModelData > surfaces;
  surfaces.reserve(surfaceCount);
  for (uint i = 0; i < surfaceCount; ++i) {
    const auto surface = reader.Next();
    CCubeSurface::ReadData(surface, nullptr, materialCount);
    surfaces.emplace_back(surface);
  }

  const auto arrays = PrepareModelArrays(positions, normals, colors, texCoords, packedTexCoords,
                                         (reader.GetFlags() & 2) != 0);
  mModelInstance =
      rs_new CCubeModel(surfaces, &mMatSets.front().mTextures, mMatSets.front().mData, arrays,
                        reader.GetBounds(), (reader.GetFlags() >> 1) & 1, true, -1);
  size_t memory = mResourceSize + sizeof(CModel) + sizeof(CCubeModel) +
                  mMatSets.capacity() * sizeof(SShader) + mModelInstance->GetSurfaceStorageSize();
  for (const auto& shader : mMatSets) {
    memory += shader.mTextures.capacity() * sizeof(TCachedToken< CTexture >);
  }
  if (memory > std::numeric_limits< uint >::max()) {
    Log.fatal("CMDL memory accounting overflow");
  }
  mDataLen = memory;
#else
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
  mMatSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; ++i) {
    mMatSets.push_back(SShader(MemoryFromPartData(dataCur, secSizeCur)));
    SShader& shader = mMatSets.back();
    CCubeModel::MakeTexturesFromMats(shader.mData, shader.mTextures, store, true);
    mDataLen += shader.mTextures.size() * 12;
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
  mSurfaces.reserve(surfaceCount);

  for (uint i = 0; i < surfaceCount; ++i) {
    mSurfaces.push_back(MemoryFromPartData(dataCur, secSizeCur));
  }

  mModelInstance = rs_new CCubeModel(
      &mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions, normals,
      vtxColors, floatUvs, shortUvs, *reinterpret_cast< const CAABox* >(dataPtr + 0xc),
      visorFlags ? 1 : 0, true, -1);
#endif
  sThisFrameList = this;
  if (mNext != nullptr) {
    mNext->mPrev = this;
  }
#if !defined(TARGET_PC)
  mDataLen += mSurfaces.size() * 4;
#endif
  sTotalMemory += mDataLen;
  DCFlushRange(mData.get(), length);
}

CModel::~CModel() {
  RemoveFromList();
  RemoveFromTotal(mDataLen);
  const int frame = CGraphics::GetFrameCounter();
  if (mLastFrame == frame) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mData.release());
  } else if (mLastFrame == frame - 1) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_ThisFrame,
                                          mData.release());
  }
}

void CModel::Draw(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(TModelPositions(), TModelNormals(), kSS_All);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->Draw(flags);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(TModelPositions(), TModelNormals(), kSS_Unsorted);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->DrawNormal(flags);
}

void CModel::DrawSortedParts(const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(TModelPositions(), TModelNormals(), kSS_Sorted);
  }

  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->DrawAlpha(flags);
}

void CModel::Draw(TModelPositions positions, TModelNormals normals,
                  const CModelFlags& flags) const {
  if (flags.GetOtherFlags() & CModelFlags::kF_DrawNormal) {
    mModelInstance->DrawNormal(positions, normals, kSS_All);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.GetShaderSet());
  mModelInstance->Draw(positions, normals, flags);
}

void CModel::VerifyCurrentShader(int shader) const {
  if (shader >= mMatSets.size()) {
    shader = 0;
  }
  if (shader == mCurrentMatxIdx) {
    if (mLastMaterialFrame != 0 && mLastMaterialFrame <= sFrameCounter) {
      for (int i = 0; i < mMatSets.size(); ++i) {
        if (i != shader) {
          mMatSets[i].UnlockTextures();
        }
      }
      mLastMaterialFrame = 0;
    }
  } else {
    mCurrentMatxIdx = shader;
    SShader& material = mMatSets[shader];
    mModelInstance->RemapMaterialData(material.mData, &material.mTextures);
    if (mMatSets.size() > 1) {
      mLastMaterialFrame = sFrameCounter + 2;
    }
  }
}

const CFactoryFnReturn FModelFactory(const SObjectTag& tag, const rstl::auto_ptr< uchar >& ptr,
                                     int len, const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  GXInvalidateVtxCache();
  return rs_new CModel(ptr, len, *pool);
}

const float* CModel::GetPositions() const {
  return static_cast< const float* >(mModelInstance->GetPositions());
}

const float* CModel::GetNormals() const {
  return static_cast< const float* >(mModelInstance->GetNormals());
}

void CModel::Touch(int shader) const {
  MoveToThisFrameList();
  VerifyCurrentShader(shader);
  if (mModelInstance->TryLockTextures()) {
    const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
    for (AUTO(it, textures.begin()); it != textures.end(); ++it) {
      if (it->GetObject()) {
        (void)it->GetObject()->LoadToMRAM();
      }
    }
  }
}

bool CModel::IsLoaded(int shader) const {
  VerifyCurrentShader(shader);

  const rstl::vector< TCachedToken< CTexture > >& textures = mModelInstance->GetTextures();
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
    mNext = sThisFrameList;
    mNext->mPrev = const_cast< CModel* >(this);
  }

  sThisFrameList = const_cast< CModel* >(this);
}

void CModel::RemoveFromList() const {
  if (mPrev != nullptr) {
    mPrev->mNext = mNext;
  } else if (this == sThisFrameList) {
    sThisFrameList = mNext;
  } else if (this == sOneFrameList) {
    sOneFrameList = mNext;
  } else if (this == sTwoFrameList) {
    sTwoFrameList = mNext;
  }

  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }

  mPrev = nullptr;
  mNext = nullptr;
}

void CModel::SShader::UnlockTextures() {
  for (AUTO(it, mTextures.begin()); it != mTextures.end(); ++it) {
    it->Unlock();
  }
}

void CModel::FrameDone() {
  ++sFrameCounter;
  if (!sIsTextureTimeoutEnabled) {
    return;
  }

  for (CModel* model = sTwoFrameList; model != nullptr;) {
    CModel* next = model->mNext;
    model->VerifyCurrentShader(0);
    for (AUTO(it, model->mMatSets.begin() + 1); it != model->mMatSets.end(); ++it) {
      it->UnlockTextures();
    }
    model->mModelInstance->UnlockTextures();
    model->mPrev = model->mNext = nullptr;
    model = next;
  }

  sTwoFrameList = sOneFrameList;
  sOneFrameList = sThisFrameList;
  sThisFrameList = nullptr;
}

void CModel::DisableTextureTimeout() { sIsTextureTimeoutEnabled = false; }

void CModel::EnableTextureTimeout() { sIsTextureTimeoutEnabled = true; }

#if defined(TARGET_PC)
uint CModel::GetDataSize() const { return mResourceSize; }
#else
uint CModel::GetDataSize() const { return mDataLen; }

rstl::auto_ptr< uchar > CModel::GetData() { return rstl::auto_ptr< uchar >(mData.get()); }

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
      reinterpret_cast< uintptr_t >(data) - reinterpret_cast< uintptr_t >(mData.release());
  mData = data;
  for (int i = 0; i < mMatSets.size(); ++i) {
    RemapPointer(mMatSets[i].mData, offset);
  }

  const CCubeModel::ModelInstance& instance = mModelInstance->GetModelInstance();
  const uchar* positions = static_cast< const uchar* >(instance.GetVertexPointer());
  const uchar* normals = static_cast< const uchar* >(instance.GetNormalPointer());
  const uchar* colors = static_cast< const uchar* >(instance.GetColorPointer());
  const uchar* uvs = static_cast< const uchar* >(instance.GetTCPointer());
  const uchar* packedUvs = static_cast< const uchar* >(instance.GetPackedTCPointer());
  const CAABox bounds = mModelInstance->GetBoundingBox();
  uchar flags = mModelInstance->GetModelFlags();
  bool texturesLoaded = mModelInstance->AreTexturesLoaded();
  const int index = mModelInstance->GetModelIndex();
  RemapPointer(positions, offset);
  RemapPointer(normals, offset);
  RemapPointer(colors, offset);
  RemapPointer(uvs, offset);
  RemapPointer(packedUvs, offset);
  for (int i = 0; i < mSurfaces.size(); ++i) {
    RemapPointer(mSurfaces[i], offset);
  }

  mModelInstance =
      rs_new CCubeModel(&mSurfaces, &mMatSets.front().mTextures, mMatSets.front().mData, positions,
                        normals, colors, uvs, packedUvs, bounds, flags, texturesLoaded, index);
  MoveToThisFrameList();
}
#endif

void CModel::UpdateLastFrame() const { mLastFrame = CGraphics::GetFrameCounter(); }
