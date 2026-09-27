#include "MetroidPrime/CGameArea.hpp"

#include "MetroidPrime/CMemoryDrawEnum.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CScriptLayerManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAreaAttributes.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "WorldFormat/CAreaBspTree.hpp"
#include "WorldFormat/CAreaOctTree.hpp"
#include "WorldFormat/CPVSAreaSet.hpp"

#if defined(TARGET_PC)
#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Metaforce/CModelSectionReader.hpp"
#include "Metaforce/Endian.hpp"
#include <borealis/log.hpp>

namespace {
constexpr borealis::Log Log{"CGameArea"};
} // namespace
#endif

#define ROUND_UP_32(val) (((val) + 31) & ~31)

struct SMREAHeader {
  uint magic;
  uint version;
  CTransform4f transform;
  int modelCount;
  int sectionCount;
  int geometrySection;
  int scriptSection;
  int collisionSection;
  int unknownSection;
  int lightSection;
  int visibilitySection;
  int pathSection;
  int renderOctreeSection;
};

float CGameArea::skEntityThinkDisableDelayOnOcclusion = 5.f;

IGameArea::~IGameArea() {}

int CGameArea::VerifyHeader() const {
  if (!mMreaSecBufs.empty()) {
    const int* header = reinterpret_cast< const int* >(mMreaSecBufs.front().first.get());
    if (CBasics::SwapBytes(header[0]) == 0xdeadbeef && CBasics::SwapBytes(header[1]) >= 12 &&
        CBasics::SwapBytes(header[1]) <= 15) {
      return CBasics::SwapBytes(header[1]);
    }
  }
  return 0;
}

int CGameArea::GetPreConstructedSize() const {
  return sizeof(CGameArea) + mAttachedAreaIndices.size() * sizeof(ushort) +
         (mDeps1.size() + mDeps2.size()) * sizeof(rstl::pair< uint, uint >) +
         mLayerDepOffsets.size() * sizeof(uint) + sizeof(CToken) + mTokens.capacity();
}

int CGameArea::GetPostConstructedSize() const {
  int surfaceSize = 0;
  for (rstl::vector< CMetroidModelInstance >::const_iterator it =
           mPostConstructed->mInsts.begin();
       it != mPostConstructed->mInsts.end(); ++it) {
    surfaceSize += it->GetSurfaces().size() * sizeof(void*);
  }
  return surfaceSize + (mUnk - mPostConstructed->x1104_) +
         (mPostConstructed->mInsts.size() * sizeof(CMetroidModelInstance)) +
         (mPostConstructed->mLightsA.size() * sizeof(CWorldLight)) +
         (mPostConstructed->mLightsB.size() * sizeof(CWorldLight)) +
         (mPostConstructed->mLayerOffsets.size() * sizeof(rstl::pair< int, int >)) +
         (sizeof(CAreaObjectList));
}

CGameArea::CPostConstructed::CPostConstructed()
: mCollision(nullptr)
, mCollisionSize(0)
, mBspTree(nullptr)
, mPvs(nullptr)
, mPvsEntityMap(SPVSActorInfo(0xffff, kInvalidUniqueId))
, mPvsVersion(0)
, mPathArea(nullptr)
, mAreaObjectList(nullptr)
, mAreaFog(nullptr)
, mSclyBuf(nullptr)
, mSclySize(0)
, mFirstMatPtr(nullptr)
, mAreaAttributes(nullptr)
, mOcclusionState(kOS_Occluded)
, x10e0_(0)
, mOccludedTime(skEntityThinkDisableDelayOnOcclusion)
, x10e8_(-1)
, mFirstMatSection(0)
, x1100_(0)
, x1104_(0)
, x1108_24_(false)
, mModelsConstructed(false)
, x1108_26_(false)
, mOcclusionPinged(false)
, mPvsHasActors(false)
, x1108_30_(false)
, mThermalCurrent(0.f)
, mThermalSpeed(0.f)
, mThermalTarget(0.f)
, mWorldLightingLevel(1.f)
, mXraySpeed(0.f)
, mXrayTarget(1.f)
, mWeaponWorldLightingSpeed(0.f)
, mWeaponWorldLightingTarget(1.f)
, mPlayerActorsLoading(0) {}

CGameArea::CGameArea(CInputStream& in, int idx, int mlvlVersion)
: mSelfIdx(idx)
, mNameSTRG(in.ReadLong())
, mTransform(in)
, mInvTransform(mTransform.GetInverse())
, mAabb(in)
, mMrea(in.ReadLong())
, mAreaId(mlvlVersion > 15 ? in.ReadLong() : -1)
, mAttachedAreaIndices(in)
, mDeps1(in)
, mDeps2(in)
, mTotalResourcesSize(0)
, mPostConstructedFlag(false)
, mActive(true)
, mTokensReady(false)
, mLoadPaused(false)
, mValidated(false)
, mPhase(kP_LoadHeader)
, mUnk(0)
, mSecCount(0)
, mMreaDataOffset(0)
, mPostConstructed(nullptr)
, mNext(nullptr)
, mPrev(nullptr)
, mCurChain(-1) {
  mAabb = mAabb.GetTransformedAABox(mTransform);
  if (mlvlVersion > 13) {
    mLayerDepOffsets = rstl::vector< uint >(in);
  } else {
    mLayerDepOffsets.reserve(1);
    mLayerDepOffsets.push_back(0);
  }
  int dockCount = in.Get< int >();
  mDocks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    mDocks.push_back(Dock(in, mTransform));
  }
  ClearTokenList();
  for (int i = 0; i < mTokens.size(); ++i) {
    mTotalResourcesSize +=
        gpResourceFactory->ResourceSize(SObjectTag(mDeps2[i].second, mDeps2[i].first));
  }
  mTotalResourcesSize += gpResourceFactory->ResourceSize(SObjectTag('MREA', mMrea));
  CMemoryDrawEnum::AddWorldMemory(GetPreConstructedSize());
}

CGameArea::~CGameArea() {
  if (mPostConstructedFlag) {
    CMemoryDrawEnum::SubtractWorldMemory(GetPostConstructedSize());
  }
  CMemoryDrawEnum::SubtractWorldMemory(GetPreConstructedSize());
  if (mPostConstructedFlag) {
    RemoveStaticGeometry();
  } else {
    while (!Invalidate(nullptr)) {
    }
  }
}

void CGameArea::ClearTokenList() {
  if (mTokens.capacity() == 0) {
    mTokens.reserve(mDeps2.size());
  } else {
    mTokens.clear();
  }
  mTokensReady = false;
}

void CGameArea::VerifyTokenList(CStateManager& mgr) {
  if (mTokens.empty()) {
    ClearTokenList();
    const CScriptLayerManager& layers = *mgr.WorldLayerState();
    if (!mDeps2.empty()) {
      for (int layer = mLayerDepOffsets.size() - 1; layer >= 0; --layer) {
        if (layers.IsLayerActive(mSelfIdx, layer)) {
          int start = mLayerDepOffsets[layer];
          int end = layer + 1 < mLayerDepOffsets.size() ? mLayerDepOffsets[layer + 1]
                                                           : mDeps2.size();
          for (int i = start; i < end; ++i) {
            const rstl::pair< uint, uint >& dep = mDeps2[i];
            mTokens.push_back(gpSimplePool->GetObj(SObjectTag(dep.second, dep.first)));
          }
        }
      }
    }
  }
}

static rstl::pair< rstl::auto_ptr< char >, int > GetScriptingMemoryAlways(const IGameArea& area) {
  SObjectTag tag('MREA', area.IGetAreaAssetId());
  rstl::auto_ptr< char > headerBuffer = rs_new char[0x60];
  const int* header = reinterpret_cast< const int* >(headerBuffer.get());
  rstl::single_ptr< CInputStream > headerStream(
      gpResourceFactory->GetResLoader().LoadNewResourcePartSync(tag, 0, 0x60, headerBuffer.get()));
  if (headerStream.get()) {
    int magic = CBasics::SwapBytes(header[0]);
    int version = CBasics::SwapBytes(header[1]);
    if (magic == 0xdeadbeef && version >= 12 && version <= 15) {
      int scriptSection = CBasics::SwapBytes(header[17]);
#if TARGET_LITTLE_ENDIAN
      int sectionCount = CBasics::SwapBytes(header[15]);
#else
      int sectionCount = header[15];
#endif
      int sizesLength = ROUND_UP_32(sectionCount * 4);
      rstl::single_ptr< CInputStream > sizesStream(
          gpResourceFactory->GetResLoader().LoadNewResourcePartSync(tag, 0x60, sizesLength,
                                                                    nullptr));
      rstl::vector< int > sizes;
      sizes.reserve(sectionCount);
      for (int i = 0; i < sectionCount; ++i) {
        sizes.push_back(sizesStream->Get< int >());
      }
      int offset = 0x60 + sizesLength;
      for (int i = 0; i < scriptSection; ++i) {
        offset += sizes[i];
      }
      int size = sizes[scriptSection];
      rstl::auto_ptr< char > buffer = rs_new char[size];
      rstl::single_ptr< CInputStream > scriptStream(
          gpResourceFactory->GetResLoader().LoadNewResourcePartSync(tag, offset, size,
                                                                    buffer.get()));
      return rstl::pair< rstl::auto_ptr< char >, int >(buffer, size);
    }
  }
  return rstl::pair< rstl::auto_ptr< char >, int >(nullptr, 0);
}

void CGameArea::FillInStaticGeometry() {
#if defined(TARGET_PC)
  if (mPostConstructed->mModelsConstructed) {
    return;
  }
  int section = mPostConstructed->mFirstMatSection;
  auto nextSection = [&]() -> std::span< uchar > {
    if (section < 0 || section >= mMreaSecBufs.size()) {
      Log.fatal("Missing MREA model section");
    }
    const auto& part = mMreaSecBufs[section++];
    if (part.second < 0 || (part.second && part.first.null())) {
      Log.fatal("Invalid MREA model section");
    }
    return part.second ? std::span(reinterpret_cast< uchar* >(part.first.get()),
                                   static_cast< size_t >(part.second))
                       : std::span< uchar >{};
  };
  const TModelData materials = nextSection();
  const uint materialCount = ValidateModelMaterials(materials);
  mPostConstructed->mFirstMatPtr = materials.data();
  mPostConstructed->mInsts.clear();
  const int modelCount = mPostConstructed->mInsts.capacity();
  for (int model = 0; model < modelCount; ++model) {
    const auto header = nextSection();
    if (header.size() < 76) {
      Log.fatal("Truncated MREA model header");
    }
    const auto positions = nextSection();
    const auto normals = nextSection();
    const auto colors = nextSection();
    const auto texCoords = nextSection();
    const auto packedTexCoords = nextSection();
    const auto surfaceInfo = nextSection();
    const uint surfaceCount = ReadModelSurfaceCount(surfaceInfo, mMreaSecBufs.size() - section);
    std::vector< TModelData > surfaces;
    surfaces.reserve(surfaceCount);
    for (uint i = 0; i < surfaceCount; ++i) {
      const auto surface = nextSection();
      CCubeSurface::ReadData(surface, nullptr, materialCount);
      surfaces.emplace_back(surface);
    }
    if (surfaceCount != 0) {
      const auto arrays = PrepareModelArrays(positions, normals, colors, texCoords, packedTexCoords,
                                             (read_bits< uint >(header.data()) & 1) != 0);
      mPostConstructed->mInsts.push_back(
          CMetroidModelInstance(header, materials, arrays, surfaces));
    }
  }
#else
  AUTO(section, mMreaSecBufs.begin() + mPostConstructed->mFirstMatSection);
  mPostConstructed->mFirstMatPtr = reinterpret_cast< const u8* >(section->first.get());
  mPostConstructed->mInsts.clear();
  ++section;
  const int modelCount = mPostConstructed->mInsts.capacity();
  rstl::vector< void* > surfaces;
  for (int model = 0; model < modelCount; ++model) {
    const void* header = section->first.get();
    const void* positions = (++section)->first.get();
    const void* normals = (++section)->first.get();
    const void* colors = (++section)->first.get();
    const void* texCoords = (++section)->first.get();
    const void* packedTexCoords = (++section)->first.get();
    const uint surfaceCount =
        CBasics::SwapBytes(*reinterpret_cast< const uint* >((++section)->first.get()));
    ++section;
    if (surfaceCount != 0) {
      surfaces.reserve(surfaceCount);
      for (uint surface = 0; surface < surfaceCount; ++surface) {
        surfaces.push_back(section->first.get());
        ++section;
      }
      mPostConstructed->mInsts.push_back(
          CMetroidModelInstance(header, mPostConstructed->mFirstMatPtr, positions, normals,
                                colors, texCoords, packedTexCoords, surfaces));
      surfaces.clear();
    }
  }
#endif
  mPostConstructed->mModelsConstructed = true;
}

static inline CVector3f SwapVectorBytes(CVector3f vec) {
  return CVector3f(CBasics::SwapBytes(vec.GetX()), CBasics::SwapBytes(vec.GetY()),
                   CBasics::SwapBytes(vec.GetZ()));
}

void CGameArea::PostConstructArea() {
  const int version = VerifyHeader();
  rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > >::const_iterator section =
      mMreaSecBufs.begin();
  const SMREAHeader* header = reinterpret_cast< const SMREAHeader* >(section->first.get());
  // Retail retains these transform comparisons with their results discarded.
  for (int i = 0; i < 3; ++i) {
    CVector3f row = SwapVectorBytes(header->transform.GetRow(i));
    close_enough(mTransform.GetRow(i), row, 0.001f);
  }
  CVector3f translation = SwapVectorBytes(header->transform.GetTranslation());
  close_enough(mTransform.GetTranslation(), translation, 0.001f);

  const int modelCount = CBasics::SwapBytes(header->modelCount);
  section += 2;
  int firstGeometry = section - mMreaSecBufs.begin();
  mPostConstructed->mFirstMatSection = firstGeometry;
  ++section;
  mPostConstructed->mInsts.reserve(modelCount);
  for (int i = 0; i < modelCount; ++i) {
#if TARGET_LITTLE_ENDIAN
    int surfaces = CBasics::SwapBytes(*reinterpret_cast< const int* >((section + 6)->first.get()));
#else
    int surfaces = *reinterpret_cast< const int* >((section + 6)->first.get());
#endif
    section += 7;
    section += surfaces;
  }

  long geometryEnd = section - mMreaSecBufs.begin();
  if (version >= 15 && CBasics::SwapBytes(header->renderOctreeSection) != -1) {
    rstl::auto_ptr< const u8 > buffer(reinterpret_cast< const u8* >(section->first.get()));
    buffer.release();
    mPostConstructed->mOctTree = CAreaRenderOctTree(buffer);
    ++section;
  }

  mPostConstructed->mSclyBuf = section->first.get();
  mPostConstructed->mSclyBuf.release();
  mPostConstructed->mSclySize = section->second;
  ++section;

  char* collisionData = section->first.get();
  ++collisionData;
  while (reinterpret_cast< uintptr_t >(collisionData) & 3) {
    ++collisionData;
  }
  uint collisionSize = CBasics::SwapBytes(*reinterpret_cast< const uint* >(collisionData));
  CAreaOctTree* collision = nullptr;
  bool collisionOwned = false;
  CAreaOctTree::MakeFromMemory(collisionData + 4, collisionSize, &collision, &collisionOwned);
  mPostConstructed->mCollision = collision;
  if (!collisionOwned) {
    mPostConstructed->mCollision.release();
  }
  mPostConstructed->mCollisionSize = collisionSize;
  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    mPostConstructed->mBspTree = rs_new CAreaBspTree(stream, mTransform);
  }

  if (version > 6) {
    ++section;
    CMemoryInStream stream(section->first.get(), section->second);
    const uint magic = stream.ReadLong();
    const bool twoLayers = magic == 0xbabedead;
    int count = twoLayers ? stream.ReadLong() : magic;
    mPostConstructed->mLightsA.reserve(count);
    mPostConstructed->mGfxLightsA.reserve(count);
    for (int i = 0; i < count; ++i) {
      mPostConstructed->mLightsA.push_back(CWorldLight(stream));
      mPostConstructed->mGfxLightsA.push_back(
          mPostConstructed->mLightsA[i].GetAsCGraphicsLight());
    }
    if (twoLayers) {
      const int countB = stream.Get< int >();
      if (countB != 0) {
        mPostConstructed->mLightsB.reserve(countB);
        mPostConstructed->mGfxLightsB.reserve(countB);
        for (int i = 0; i < countB; ++i) {
          mPostConstructed->mLightsB.push_back(CWorldLight(stream));
          mPostConstructed->mGfxLightsB.push_back(
              mPostConstructed->mLightsB[i].GetAsCGraphicsLight());
        }
      }
    }

    const CPostConstructed* post = mPostConstructed.get();
    if (post->mLightsB.size() == 0) {
      mPostConstructed->mLightsB = mPostConstructed->mLightsA;
      mPostConstructed->mGfxLightsB = mPostConstructed->mGfxLightsA;
    }
  }

  if (version > 7) {
    ++section;
    int size = section->second;
    if (size > 64) {
      const char* const buffer = section->first.get();
      CMemoryInStream stream(buffer, size);
      if (stream.ReadLong() == 'VISI') {
        int pvsVersion = stream.ReadLong();
        mPostConstructed->mPvsVersion = pvsVersion;
        if (mPostConstructed->mPvsVersion == 2) {
          mPostConstructed->mPvsHasActors = stream.ReadBool();
          mPostConstructed->x1108_30_ = stream.ReadBool();
          mPostConstructed->mPvs =
              CPVSAreaSet::MakeAreaSet(buffer + stream.GetReadPosition(),
                                       size - stream.GetReadPosition())
                  .release();
        }
      }
    }
  }

  if (version > 9) {
    ++section;
    CMemoryInStream stream(section->first.get(), section->second);
    CAssetId pathId = stream.ReadLong();
    if (pathId != kInvalidAssetId) {
      mPostConstructed->mPathToken =
          TLockedToken< CPFArea >(gpSimplePool->GetObj(SObjectTag('PATH', pathId)));
      mPostConstructed->mPathArea = **mPostConstructed->mPathToken;
      mPostConstructed->mPathArea->SetTransform(mTransform);
    }
  }

#if defined(TARGET_PC)
  mPostConstructed->x1108_24_ = true;
#else
  int firstARAM = firstGeometry;
  int sectionCount = mMreaSecBufs.size();
  for (; firstARAM < sectionCount; ++firstARAM) {
    if (mMreaSecBufs[firstARAM].first.owner()) {
      break;
    }
  }
  int lastARAM = geometryEnd;
  for (; firstARAM < lastARAM; --lastARAM) {
    if (mMreaSecBufs[lastARAM].first.owner()) {
      break;
    }
  }
  if (firstARAM < lastARAM) {
    mPostConstructed->x10e8_ = firstARAM;
    int bufferCount = 0;
    for (int i = firstARAM; i < lastARAM; ++i) {
      if (mMreaSecBufs[i].first.owner()) {
        ++bufferCount;
      }
    }
    mPostConstructed->mTokens.reserve(bufferCount);
    for (int part = firstARAM; part < lastARAM;) {
      int start = part;
      uint size = mMreaSecBufs[part++].second;
      for (; part < lastARAM && !mMreaSecBufs[part].first.owner(); ++part) {
        size += mMreaSecBufs[part].second;
      }
      mPostConstructed->mTokens.push_back(rstl::pair< CARAMToken, int >(
          CARAMToken(mMreaSecBufs[start].first.release(), size, 1), part - start));
      mPostConstructed->x1104_ += size;
      if (GetOcclusionState() == kOS_Occluded) {
        CARAMToken& token = mPostConstructed->mTokens.back().first;
        token.LoadToARAM();
        if (token.GetStatus() != CARAMToken::kS_One) {
          mPostConstructed->x1100_ += size;
        }
      }
    }
    bool modelsInMRAM = GetOcclusionState() != kOS_Occluded;
    mPostConstructed->x1108_24_ = modelsInMRAM;
  }
#endif

  mPostConstructed->mAreaObjectList = rs_new CAreaObjectList(mSelfIdx);
  mPostConstructed->mAreaFog = rs_new CAreaFog;
  mPostConstructedFlag = true;
  CMemoryDrawEnum::AddWorldMemory(GetPostConstructedSize());

  CMemoryInStream stream(mPostConstructed->mSclyBuf.get(), GetScriptingSize());
  if (stream.ReadLong() == 'SCLY') {
    stream.ReadLong();
    int layerCount = stream.ReadLong();
    rstl::vector< int > sizes;
    sizes.reserve(layerCount);
    for (int i = 0; i < layerCount; ++i) {
      sizes.push_back(stream.Get< int >());
    }
    const int firstLayerOffset = (layerCount + 3) * 4;
    mPostConstructed->mLayerOffsets.reserve(layerCount);
    for (int i = 0; i < layerCount; ++i) {
      int offset = i == 0 ? firstLayerOffset
                          : mPostConstructed->mLayerOffsets[i - 1].first +
                                mPostConstructed->mLayerOffsets[i - 1].second;
      mPostConstructed->mLayerOffsets.push_back(rstl::pair< int, int >(offset, sizes[i]));
    }
  } else {
    mPostConstructed->mLayerOffsets.reserve(1);
    mPostConstructed->mLayerOffsets.push_back(
        rstl::pair< int, int >(0, GetScriptingSize()));
  }
}

void CGameArea::Validate(CStateManager& mgr) {
  if (!mPostConstructedFlag) {
    while (StartStreamingMainArea()) {
    }
    for (AUTO(it, mLoadTransactions.begin()); it != mLoadTransactions.end(); ++it) {
      if (it->get() != nullptr) {
        (*it)->WaitUntilComplete();
      }
    }
    if (mTokens.empty()) {
      VerifyTokenList(mgr);
      for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
        it->Lock();
      }
      for (AUTO(it, mTokens.begin()); it != mTokens.end(); ++it) {
        it->GetObj();
      }
      mTokensReady = true;
    }
    mLoadTransactions.clear();
    mPhase = kP_LoadHeader;
    PostConstructArea();
    if (mSelfIdx != kInvalidAreaId) {
      mgr.World()->MoveAreaToChain3(mSelfIdx);
    }
    LoadScriptObjects(mgr);
    if (mPostConstructed->mPvs.get() != nullptr &&
        mPostConstructed->mPvsHasActors) {
      for (int i = 0; i < mPostConstructed->mPvs->GetNumActors(); ++i) {
        const CPostConstructed* post = mPostConstructed.get();
        uint editorId = post->mPvs->GetEntityIdByIndex(i) | (mSelfIdx.Value() << 16);
        TUniqueId id = mgr.GetIdForScript(editorId);
        if (id != kInvalidUniqueId) {
          const CPVSAreaSet* pvs = mPostConstructed->mPvs.get();
          mPostConstructed->mPvsEntityMap[id.Value()] =
              SPVSActorInfo(i + (pvs->GetNumFeatures() - pvs->GetNumActors()), id);
        }
      }
    }
    mValidated = true;
    mgr.AreaLoaded(GetId());
  }
}

void CGameArea::LoadScriptObjects(CStateManager& mgr) {
  rstl::vector< TEditorId > ids;
  const CScriptLayerManager& layers = *mgr.WorldLayerState();
  int count = layers.GetAreaLayerCount(mSelfIdx);
  for (int i = 0; i < count; ++i) {
    TLayerId layer(i);
    if (layers.IsLayerActive(mSelfIdx, layer)) {
      rstl::pair< const uchar*, int > buffer = GetLayerScriptBuffer(layer);
      CMemoryInStream stream(buffer.first, buffer.second);
      mgr.LoadScriptObjects(GetId(), stream, ids);
    }
  }
  mgr.InitScriptObjects(ids);
}

void CGameArea::StartStreamIn(CStateManager& mgr) {
  bool fullyInitialized = mgr.IsFullyInitialized();
  if (!mPostConstructedFlag && !mLoadPaused) {
    VerifyTokenList(mgr);
    if (!mTokensReady) {
      int notLoaded = 0;
      for (int i = 0; i < mTokens.size(); ++i) {
        CToken& token = mTokens[i];
        if (token.IsLoaded()) {
          token.Lock();
          if (token.GetReferenceType() == 'TXTR') {
            TToken< CTexture > textureToken(token);
            CTexture* texture = textureToken.GetT();
            texture->MakeSwappable();
            if (fullyInitialized) {
              texture->LoadToARAM();
            }
          }
        } else {
          gpResourceFactory->GetResLoader().FindResource(SObjectTag('MREA', mMrea));
          token.Lock();
          ++notLoaded;
        }
      }
      if (notLoaded == 0) {
        mTokensReady = true;
      } else {
        return;
      }
    }
    StartStreamingMainArea();
    if (mPhase == kP_WaitForFinish) {
      CullDeadAreaRequests();
      if (mLoadTransactions.empty()) {
        Validate(mgr);
      }
    }
  }
}

void CGameArea::CullDeadAreaRequests() {
  while (!mLoadTransactions.empty() && mLoadTransactions.front()->IsComplete()) {
    mLoadTransactions.pop_front();
  }
}

void CGameArea::KillmAreaData() {
  mMreaSecBufs = rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > >();
}

bool CGameArea::Invalidate(CStateManager* mgr) {
  if (!mPostConstructedFlag) {
    ClearTokenList();
    for (AUTO(it, mLoadTransactions.begin()); it != mLoadTransactions.end();) {
      AUTO(cur, it);
      ++it;
      if (!(*cur)->IsComplete()) {
        (*cur)->PostCancelRequest();
      } else {
        mLoadTransactions.erase(cur);
      }
    }
    if (!mLoadTransactions.empty()) {
      return false;
    }
    mPostConstructed = nullptr;
    mPhase = kP_LoadHeader;
    KillmAreaData();
    return true;
  }
  if (mgr != nullptr) {
    mgr->PrepareAreaUnload(GetId());
  }
  CMemoryDrawEnum::SubtractWorldMemory(GetPostConstructedSize());
  RemoveStaticGeometry();
  mPostConstructed = nullptr;
  mPostConstructedFlag = false;
  mValidated = false;
  mPhase = kP_LoadHeader;
  CullDeadAreaRequests();
  KillmAreaData();
  mUnk = 0;
  ClearTokenList();
  if (mgr != nullptr) {
    mgr->AreaUnloaded(GetId());
  }
  return true;
}

char* CGameArea::AllocNewAreaData(int offset, int size) {
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  rstl::pair< rstl::auto_ptr< char >, int > data(buffer, size);
  mMreaSecBufs.push_back(data);
  SObjectTag tag('MREA', mMrea);
  mLoadTransactions.push_back(rstl::auto_ptr< CDvdRequest >(
      gpResourceFactory->GetResLoader().LoadResourcePartAsync(tag, offset, size, buffer)));
  return buffer;
}

int CGameArea::GetNumPartSizes() const {
  return CBasics::SwapBytes(
      reinterpret_cast< const int* >(mMreaSecBufs.front().first.get())[15]);
}

bool CGameArea::ReloadAllUnloadedTextures() {
  bool finished = true;
  for (int i = 0; i < mTokens.size(); ++i) {
    CToken& token = mTokens[i];
    if (token.GetReferenceType() == 'TXTR' && token.IsLoaded() && token.IsLocked()) {
      TToken< CTexture > textureToken(token);
      CTexture* texture = textureToken.GetT();
      texture->MakeSwappable();
      if (!texture->LoadToMRAM()) {
        finished = false;
      }
    }
  }
  return finished;
}

bool CGameArea::UnloadAllloadedTextures() {
  bool finished = true;
  for (int i = 0; i < mTokens.size(); ++i) {
    CToken& token = mTokens[i];
    if (token.GetReferenceType() == 'TXTR' && token.IsLoaded() && token.IsLocked()) {
      TToken< CTexture > textureToken(token);
      CTexture* texture = textureToken.GetT();
      texture->MakeSwappable();
      texture->LoadToARAM();
      if (texture->IsARAMTransferInProgress()) {
        finished = false;
      }
    }
  }
  return finished;
}

bool CGameArea::StartStreamingMainArea() {
  if (mPostConstructedFlag) {
    return false;
  }
  switch (mPhase) {
  case kP_LoadHeader: {
    mMreaSecBufs.reserve(2);
    AllocNewAreaData(0, 96);
    mPostConstructed = rs_new CPostConstructed();
    mPhase = kP_LoadSecSizes;
    break;
  }
  case kP_LoadSecSizes: {
    CullDeadAreaRequests();
    if (mLoadTransactions.empty()) {
      VerifyHeader();
      int count = GetNumPartSizes();
      AllocNewAreaData(mMreaSecBufs[0].second, ROUND_UP_32(count * 4));
      mPhase = kP_ReserveSections;
    }
    break;
  }
  case kP_ReserveSections: {
    CullDeadAreaRequests();
    if (mLoadTransactions.empty()) {
#if TARGET_LITTLE_ENDIAN
      // Decode the loaded size table once, before any section offsets are calculated.
      int* sizes = reinterpret_cast< int* >(mMreaSecBufs[1].first.get());
      for (int i = 0; i < GetNumPartSizes(); ++i) {
        sizes[i] = CBasics::SwapBytes(sizes[i]);
      }
#endif
      mMreaSecBufs.reserve(GetNumPartSizes() + 2);
      int headerSize = mMreaSecBufs[0].second;
      int sizesSize = mMreaSecBufs[1].second;
      mSecCount = 0;
      headerSize += sizesSize;
      mMreaDataOffset = headerSize;
      mPhase = kP_LoadDataSections;
    }
    break;
  }
  case kP_LoadDataSections: {
    CullDeadAreaRequests();
    int secCount = mSecCount;
    int totalSize = 0;
    int partSizes = GetNumPartSizes();
    const int* sizes = reinterpret_cast< const int* >(mMreaSecBufs[1].first.get());
    SObjectTag tag('MREA', mMrea);
    int targetSecCount = secCount;
    for (; targetSecCount < partSizes; ++targetSecCount) {
      int size = sizes[targetSecCount];
      if (targetSecCount != secCount && size + totalSize > 0x10000) {
        break;
      }
      totalSize += size;
    }
    rstl::auto_ptr< char > buffer =
        static_cast< char* >(CMemory::Alloc(totalSize, IAllocator::kHI_RoundUpLen));
    mLoadTransactions.push_back(
        rstl::auto_ptr< CDvdRequest >(gpResourceFactory->GetResLoader().LoadResourcePartAsync(
            tag, mMreaDataOffset, totalSize, buffer.get())));
    mMreaDataOffset += totalSize;
    const int firstSize = sizes[secCount];
    int offset = firstSize;
    mMreaSecBufs.push_back(rstl::pair< rstl::auto_ptr< char >, int >(buffer, firstSize));
    for (int i = secCount + 1; i < targetSecCount; ++i) {
      rstl::auto_ptr< char > section(buffer.get() + offset);
      section.release();
      int size = sizes[i];
      mMreaSecBufs.push_back(rstl::pair< rstl::auto_ptr< char >, int >(section, size));
      offset += size;
    }
    mSecCount = targetSecCount;
    if (targetSecCount == partSizes) {
      mUnk = mMreaDataOffset;
      mPhase = kP_WaitForFinish;
    }
    break;
  }
  case kP_WaitForFinish: {
    CullDeadAreaRequests();
    if (mLoadTransactions.empty()) {
      return false;
    }
    break;
  }
  }
  return true;
}

int CGameArea::SetChain(CGameArea* next, int chain) {
  if (mCurChain == chain) {
    return mCurChain;
  }
  if (mPrev != nullptr) {
    mPrev->mNext = mNext;
  }
  if (mNext != nullptr) {
    mNext->mPrev = mPrev;
  }
  mPrev = nullptr;
  mNext = next;
  if (next != nullptr) {
    next->mPrev = this;
  }
  int oldChain = mCurChain;
  mCurChain = chain;
  return oldChain;
}

bool CGameArea::TransferARAMTokensOver(EARAMTransfer mode) {
#if defined(TARGET_PC)
  return true;
#else
  if (mPostConstructed->x1108_24_) {
    return true;
  }
  bool finished = true;
  int part = mPostConstructed->x10e8_;
  for (AUTO(it, mPostConstructed->mTokens.begin());
       it != mPostConstructed->mTokens.end(); ++it) {
    if (it->first.GetStatus() != CARAMToken::kS_One) {
      mPostConstructed->x1100_ -= it->first.GetSize();
    }
    if (mode == kAT_Async && !it->first.LoadToMRAM()) {
      finished = false;
    } else if (finished) {
      char* buffer = static_cast< char* >(it->first.GetMRAMSafe());
      int offset = 0;
      for (int i = 0; i < it->second; ++i) {
        rstl::auto_ptr< char > section(buffer + offset);
        section.release();
        offset += mMreaSecBufs[part].second;
        mMreaSecBufs[part].first = section;
        ++part;
      }
    }
  }
  mPostConstructed->x1108_24_ = finished;
  return finished;
#endif
}

bool CGameArea::TransferTokensToARAM() {
#if defined(TARGET_PC)
  return true;
#else
  bool finished = true;
  int part = mPostConstructed->x10e8_;
  AUTO(it, mPostConstructed->mTokens.begin());
  rstl::auto_ptr< char > empty;
  for (; it != mPostConstructed->mTokens.end(); ++it) {
    for (int i = 0; i < it->second; ++i) {
      mMreaSecBufs[part].first = empty;
      ++part;
    }
    CARAMToken::EStatus oldStatus = it->first.GetStatus();
    it->first.LoadToARAM();
    if (oldStatus == CARAMToken::kS_One && it->first.GetStatus() != CARAMToken::kS_One) {
      mPostConstructed->x1100_ += it->first.GetSize();
    }
    if (it->first.GetStatus() >= CARAMToken::kS_Two &&
        it->first.GetStatus() <= CARAMToken::kS_Five) {
      finished = false;
    }
  }
  mPostConstructed->x1108_24_ = false;
  mPostConstructed->mModelsConstructed = false;
  return finished;
#endif
}

void CGameArea::AddStaticGeometry() {
  if (mPostConstructed->mOcclusionState != kOS_Visible) {
    mPostConstructed->x10e0_ = 0;
    mPostConstructed->mOcclusionState = kOS_Visible;
    TransferARAMTokensOver(kAT_Blocking);
    if (!mPostConstructed->mModelsConstructed) {
      FillInStaticGeometry();
    }
    const CPostConstructed* post = mPostConstructed.get();
    int areaIdx = mSelfIdx.Value();
    const CAreaRenderOctTree* tree =
        post->mOctTree.valid() ? post->mOctTree.get_ptr() : nullptr;
    gpRender->AddStaticGeometry(&mPostConstructed->mInsts, tree, areaIdx);
  }
}

void CGameArea::RemoveStaticGeometry() {
  if (mPostConstructedFlag && mPostConstructed.get() != nullptr &&
      mPostConstructed->mOcclusionState != kOS_Occluded) {
    mPostConstructed->x10e0_ = 0;
    mPostConstructed->mOcclusionState = kOS_Occluded;
    gpRender->RemoveStaticGeometry(&mPostConstructed->mInsts);
  }
}

void CGameArea::SetOcclusionState(EOcclusionState state) {
  if (mPostConstructedFlag && state != mPostConstructed->mOcclusionState) {
    if (state == kOS_Occluded) {
      mPostConstructed->x1108_26_ = true;
      mPostConstructed->x1108_27_ = false;
      RemoveStaticGeometry();
    } else {
      ReloadAllUnloadedTextures();
      AddStaticGeometry();
    }
  }
}

void CGameArea::AliveUpdate(float dt) {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    mPostConstructed->mOccludedTime += dt;
  } else {
    mPostConstructed->mOccludedTime = 0.f;
  }
  UpdateFog(dt);
  UpdateThermalVisor(dt);
  UpdateWeaponWorldLighting(dt);
}

void CGameArea::PreRender() {
  if (mPostConstructedFlag) {
    if (mPostConstructed->mOcclusionPinged) {
      mPostConstructed->mOcclusionPinged = false;
    } else {
      PingOcclusionState();
    }
  }
}

void CGameArea::PingOcclusionState() {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    if (mPostConstructed->x10e0_ < 2) {
      ++mPostConstructed->x10e0_;
      return;
    }
    mPostConstructed->x10e0_ = 3;
    if (!mPostConstructed->x1108_27_) {
      bool unloaded = UnloadAllloadedTextures();
      bool transferred = TransferTokensToARAM();
      if (unloaded && transferred) {
        mPostConstructed->x1108_27_ = true;
      }
    }
  }
  mPostConstructed->x1108_26_ = true;
}

void CGameArea::OtherAreaOcclusionChanged() {
  if (mPostConstructed->x10e0_ == 3 &&
      mPostConstructed->mOcclusionState == kOS_Occluded) {
    bool unloaded = UnloadAllloadedTextures();
    bool transferred = TransferTokensToARAM();
    mPostConstructed->x1108_27_ = unloaded && transferred;
    return;
  }
  if (mPostConstructed->mOcclusionState == kOS_Visible) {
    ReloadAllUnloadedTextures();
  }
}

bool IGameArea::Dock::IsReferenced() const { return mIsReferenced; }

int IGameArea::Dock::GetReferenceCount() const { return mReferenceCount; }

void IGameArea::Dock::SetReferenceCount(int count) {
  mReferenceCount = count;
  mIsReferenced = true;
}

TAreaId IGameArea::Dock::GetConnectedAreaId(int other) const {
  return mDockReferences.empty() ? TAreaId(-1) : mDockReferences[other].mArea;
}

int IGameArea::Dock::GetOtherDockNumber(int other) const {
  if (mDockReferences.empty()) {
    return -1;
  }
  return mDockReferences[other].mDock;
}

bool IGameArea::Dock::ShouldLoadOtherArea(int other) const {
  if (mDockReferences.empty()) {
    return false;
  }
  return mDockReferences[other].mLoadOther;
}

void IGameArea::Dock::SetShouldLoadOther(int other, bool should) {
  if (other < mDockReferences.size()) {
    mDockReferences[other].mLoadOther = should;
  }
}

bool IGameArea::Dock::GetShouldLoadOther(int other) const {
  if (other < mDockReferences.size()) {
    return mDockReferences[other].mLoadOther;
  }
  return false;
}

uchar CGameArea::CAreaObjectList::IsQualified(const CEntity& ent) {
  return mAreaId == ent.GetCurrentAreaId();
}

CGameArea::CAreaFog::CAreaFog()
: mFogMode(kRFM_None)
, mRangeCur(0.f, 1024.f)
, mRangeTarget(mRangeCur)
, mRangeDelta(0.f, 0.f)
, mColorCur(0.5f, 0.5f, 0.5f)
, mColorTarget(mColorCur)
, mColorDelta(0.f) {}

void CGameArea::CAreaFog::DisableFog() { mFogMode = kRFM_None; }

bool CGameArea::CAreaFog::IsFogDisabled() const { return mFogMode == kRFM_None; }

void CGameArea::CAreaFog::SetFogExplicit(ERglFogMode mode, const CColor& color,
                                         const CVector2f& range) {
  mFogMode = mode;
  mColorCur = mColorTarget = color.ToVector3f();
  mRangeCur = mRangeTarget = range;
}

void CGameArea::CAreaFog::FadeFog(const ERglFogMode mode, const CColor& color,
                                  const CVector2f& vec1, const float speed, const CVector2f& vec2) {
  if (mFogMode == kRFM_None) {
    mFogMode = mode;
    mColorCur = mColorTarget = color.ToVector3f();
    mRangeCur = CVector2f(vec1.GetY(), vec1.GetY());
    mRangeTarget = vec1;
  } else {
    mFogMode = mode;
    mColorTarget = color.ToVector3f();
    mRangeTarget = vec1;
  }

  mColorDelta = speed;
  mRangeDelta = vec2;
}

void CGameArea::CAreaFog::RollFogOut(const float rangeDelta, const float colorDelta,
                                     const CColor& color) {
  mRangeDelta = CVector2f(rangeDelta, rangeDelta * 2.f);
  mRangeTarget = CVector2f(4096.f, 4096.f);
  mColorDelta = colorDelta;
  mColorTarget = color.ToVector3f();
}

void CGameArea::UpdateFog(const float dt) {
  if (!mPostConstructed->mAreaFog.null()) {
    mPostConstructed->mAreaFog->Update(dt);
  }
}

void CGameArea::CAreaFog::Update(const float dt) {
  if (mFogMode == kRFM_None) {
    return;
  }

  if (!(mColorDelta > 0.f) && mRangeDelta == CVector2f(0.f, 0.f)) {
    return;
  }

  float current[5];
  float target[5];
  float result[5];
  float step[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
  step[2] = step[1] = step[0] = mColorDelta * dt;
  step[3] = dt * mRangeDelta.GetX();
  step[4] = dt * mRangeDelta.GetY();
  current[0] = mColorCur.GetX();
  target[0] = mColorTarget.GetX();
  current[1] = mColorCur.GetY();
  target[1] = mColorTarget.GetY();
  current[2] = mColorCur.GetZ();
  target[2] = mColorTarget.GetZ();
  current[3] = mRangeCur.GetX();
  current[4] = mRangeCur.GetY();
  target[3] = mRangeTarget.GetX();
  target[4] = mRangeTarget.GetY();
  int finished = 0;
  for (int i = 0; i < 5; ++i) {
    const float cur = current[i];
    const float tar = target[i];
    const float delta = tar - cur;
    const float amount = step[i];
    if (CMath::AbsF(delta) <= amount) {
      result[i] = tar;
      ++finished;
    } else {
      result[i] = cur + CMath::FastFSel(delta, amount, -amount);
    }
  }
  if (finished == 5) {
    mColorDelta = 0.f;
    mRangeDelta = CVector2f(0.f, 0.f);
    if (result[3] == result[4]) {
      mFogMode = kRFM_None;
    }
  }
  if (result[3] > result[4]) {
    result[3] = result[4];
  }
  mColorCur = CVector3f(result[0], result[1], result[2]);
  mRangeCur = CVector2f(result[3], result[4]);
}

void CGameArea::CAreaFog::SetCurrent() const {
  gpRender->SetWorldFog(mFogMode, mRangeCur.GetX(), mRangeCur.GetY(),
                        CColor(mColorCur.GetX(), mColorCur.GetY(), mColorCur.GetZ(), 1.f));
}

bool CGameArea::DoesAreaNeedSkyNow() const {
  if (mPostConstructed.get() == nullptr) {
    return false;
  }
  if (mPostConstructed->mAreaAttributes != nullptr) {
    return mPostConstructed->mAreaAttributes->GetNeedsSky();
  }
  return false;
}

EEnvFxType CGameArea::DoesAreaNeedEnvFx() const {
  if (mPostConstructed.get() == nullptr) {
    return kEFX_None;
  }
  if (mPostConstructed->mAreaAttributes == nullptr) {
    return kEFX_None;
  }
  if (mPostConstructed->mOcclusionState != kOS_Visible) {
    return kEFX_None;
  }
  return mPostConstructed->mAreaAttributes->GetEnvFxType();
}

bool CGameArea::TryTakingOutOfARAM() {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    mPostConstructed->mOcclusionPinged = true;
  }
  return TransferARAMTokensOver(kAT_Async) && ReloadAllUnloadedTextures();
}

const CTransform4f& CGameArea::IGetTM() const { return mTransform; }

CAssetId CGameArea::IGetStringTableAssetId() const { return mNameSTRG; }

uint CGameArea::IGetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }

TAreaId CGameArea::IGetAttachedAreaId(int i) const { return mAttachedAreaIndices[i]; }

bool CGameArea::IIsActive() const { return mActive; }

CAssetId CGameArea::IGetAreaAssetId() const { return mMrea; }

int CGameArea::IGetAreaSaveId() const { return mAreaId; }

rstl::pair< rstl::auto_ptr< char >, int > CGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

IGameArea::Dock::Dock(CInputStream& in, const CTransform4f& xf)
: mReferenceCount(0), mIsReferenced(false) {
  int count = in.Get< int >();
  mDockReferences.reserve(count);
  for (int i = 0; i < count; ++i) {
    TAreaId areaId(in.ReadLong());
    short dock = in.ReadLong();
    mDockReferences.push_back(SDockReference(areaId, dock, 0));
  }
  int vertCount = in.Get< int >();
  for (int i = 0; i < vertCount; ++i) {
    mPlaneVertices.push_back(xf * CVector3f(in));
  }
}

CDummyGameArea::CDummyGameArea(CInputStream& in, int idx, int mlvlVersion)
: mSelfIdx(idx), mNameSTRG(-1), mTransform(CTransform4f::Identity()) {
  mNameSTRG = in.ReadLong();
  mTransform = CTransform4f(in);
  CAABox bounds(in);
  mMrea = in.ReadLong();
  if (mlvlVersion > 15) {
    mAreaId = in.ReadLong();
  }
  mAttachedAreaIndices = rstl::vector< ushort >(in);
  {
    rstl::vector< rstl::pair< uint, uint > > deps1(in);
    rstl::vector< rstl::pair< uint, uint > > deps2(in);
  }
  if (mlvlVersion > 13) {
    rstl::vector< uint > layerDepOffsets(in);
  }
  int count = in.Get< int >();
  mDocks.reserve(count);
  for (int i = 0; i < count; ++i) {
    mDocks.push_back(Dock(in, mTransform));
  }
}

const CTransform4f& CDummyGameArea::IGetTM() const { return mTransform; }

CAssetId CDummyGameArea::IGetStringTableAssetId() const { return mNameSTRG; }

uint CDummyGameArea::IGetNumAttachedAreas() const { return mAttachedAreaIndices.size(); }

TAreaId CDummyGameArea::IGetAttachedAreaId(int idx) const { return mAttachedAreaIndices[idx]; }

bool CDummyGameArea::IIsActive() const { return true; }

CAssetId CDummyGameArea::IGetAreaAssetId() const { return mMrea; }

int CDummyGameArea::IGetAreaSaveId() const { return mAreaId; }

rstl::pair< rstl::auto_ptr< char >, int > CDummyGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

bool CGameArea::IsFinishedOccluding() const {
  if (mPostConstructed->mOcclusionState == kOS_Occluded) {
    return mPostConstructed->x1108_27_;
  }
  return true;
}

rstl::pair< const uchar*, int > CGameArea::GetLayerScriptBuffer(const TLayerId layer) const {
  if (mPostConstructedFlag) {
    const rstl::pair< int, int >& offsets = mPostConstructed->mLayerOffsets[layer.Value()];
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(mPostConstructed->mSclyBuf.get()) + offsets.first,
        offsets.second);
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

void CGameArea::SetLoadPauseState(bool paused) {
  if (!mTokensReady) {
    mLoadPaused = paused;
    if (paused) {
      for (int i = 0; i < mTokens.size(); ++i) {
        CToken& token = mTokens[i];
        if (!token.IsLoaded()) {
          token.Unlock();
        }
      }
    }
  }
}

float CGameArea::GetXRayFogDistance() {
  if (mPostConstructed->mAreaAttributes) {
    return mPostConstructed->mAreaAttributes->GetXRayFogDistance();
  }
  return 1.f;
}

void CGameArea::SetAreaAttributes(CScriptAreaAttributes* attributes) {
  mPostConstructed->mAreaAttributes = attributes;
  if (attributes) {
    mPostConstructed->mThermalCurrent = attributes->GetThermalHeat();
    mPostConstructed->mWorldLightingLevel = attributes->GetWorldLightingLevel();
  }
}

void CGameArea::SetThermalSpeedAndTarget(float speed, float target) {
  mPostConstructed->mThermalSpeed = speed;
  mPostConstructed->mThermalTarget = target;
}

void CGameArea::SetXRaySpeedAndTarget(float speed, float target) {
  mPostConstructed->mXraySpeed = speed;
  mPostConstructed->mXrayTarget = target;
}

void CGameArea::SetWeaponWorldLighting(float speed, float target) {
  mPostConstructed->mWeaponWorldLightingSpeed = speed;
  mPostConstructed->mWeaponWorldLightingTarget = target;
}

void CGameArea::UpdateThermalVisor(float dt) {
  if (mPostConstructed->mThermalSpeed == 0.f) {
    return;
  }
  float current = mPostConstructed->mThermalCurrent;
  float delta = dt * mPostConstructed->mThermalSpeed;
  if (CMath::AbsF(mPostConstructed->mThermalTarget - current) < delta) {
    current = mPostConstructed->mThermalTarget;
    mPostConstructed->mThermalSpeed = 0.f;
  } else if (mPostConstructed->mThermalTarget < current) {
    current -= delta;
  } else {
    current += delta;
  }
  mPostConstructed->mThermalCurrent = current;
}

void CGameArea::UpdateWeaponWorldLighting(float dt) {
  float lighting = mPostConstructed->mWorldLightingLevel;
  if (0.f != mPostConstructed->mXraySpeed) {
    float delta = dt * mPostConstructed->mXraySpeed;
    if (CMath::AbsF(mPostConstructed->mXrayTarget - lighting) < delta) {
      lighting = mPostConstructed->mXrayTarget;
      mPostConstructed->mWeaponWorldLightingSpeed = 0.f;
    } else if (mPostConstructed->mXrayTarget < lighting) {
      lighting -= delta;
    } else {
      lighting += delta;
    }
  }
  if (0.f != mPostConstructed->mWeaponWorldLightingSpeed) {
    float weaponLighting = mPostConstructed->mWorldLightingLevel;
    float delta = dt * mPostConstructed->mWeaponWorldLightingSpeed;
    if (CMath::AbsF(mPostConstructed->mWeaponWorldLightingTarget - lighting) < delta) {
      weaponLighting = mPostConstructed->mWeaponWorldLightingTarget;
      mPostConstructed->mWeaponWorldLightingSpeed = 0.f;
    } else if (mPostConstructed->mWeaponWorldLightingTarget < weaponLighting) {
      weaponLighting -= delta;
    } else {
      weaponLighting += delta;
    }
    if (mPostConstructed->mXraySpeed != 0.f) {
      lighting = rstl::min_val(weaponLighting, lighting);
    } else {
      lighting = weaponLighting;
    }
  }
  float epsilon = 0.00001f;
  if (!(fabs(mPostConstructed->mWorldLightingLevel - lighting) < epsilon)) {
    mPostConstructed->mWorldLightingLevel = lighting;
    CObjectList& objects = *mPostConstructed->mAreaObjectList;
    for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
      if (CActor* actor = TCastToPtr< CActor >(objects[i])) {
        actor->SetWorldLightingDirty(true);
      }
    }
  }
}

uint CGameArea::LookupPVSID(TUniqueId id) {
  return mPostConstructed->mPvsEntityMap[id.Value()].mPvsId;
}

TUniqueId CGameArea::LookupPVSUniqueID(TUniqueId id) {
  return mPostConstructed->mPvsEntityMap[id.Value()].mUniqueId;
}

uint CGameArea::Get1stPVSLightFeature(uint light) const {
  const CPVSAreaSet* pvs = mPostConstructed->mPvs.get();
  return !pvs ? -1 : pvs->GetNumFeatures() + pvs->GetNum2ndLights() + light;
}

uint CGameArea::Get2ndPVSLightFeature(uint light) const {
  const CPVSAreaSet* pvs = mPostConstructed->mPvs.get();
  if (!mPostConstructed->x1108_30_ || !pvs) {
    return -1;
  }
  const int& count = pvs->GetNumFeatures();
  return count + light;
}
