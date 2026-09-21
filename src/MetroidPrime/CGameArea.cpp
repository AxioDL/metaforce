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
  if (!x110_mreaSecBufs.empty()) {
    const int* header = reinterpret_cast< const int* >(x110_mreaSecBufs.front().first.get());
    if (CBasics::SwapBytes(header[0]) == 0xdeadbeef && CBasics::SwapBytes(header[1]) >= 12 &&
        CBasics::SwapBytes(header[1]) <= 15) {
      return CBasics::SwapBytes(header[1]);
    }
  }
  return 0;
}

int CGameArea::GetPreConstructedSize() const {
  return sizeof(CGameArea) + x8c_attachedAreaIndices.size() * sizeof(ushort) +
         (x9c_deps1.size() + xac_deps2.size()) * sizeof(rstl::pair< uint, uint >) +
         xbc_layerDepOffsets.size() * sizeof(uint) + sizeof(CToken) + xdc_tokens.capacity();
}

int CGameArea::GetPostConstructedSize() const {
  int surfaceSize = 0;
  for (rstl::vector< CMetroidModelInstance >::const_iterator it =
           x12c_postConstructed->x4c_insts.begin();
       it != x12c_postConstructed->x4c_insts.end(); ++it) {
    surfaceSize += it->GetSurfaces().size() * sizeof(void*);
  }
  return surfaceSize + (x120_unk - x12c_postConstructed->x1104_) +
         (x12c_postConstructed->x4c_insts.size() * sizeof(CMetroidModelInstance)) +
         (x12c_postConstructed->x60_lightsA.size() * sizeof(CWorldLight)) +
         (x12c_postConstructed->x80_lightsB.size() * sizeof(CWorldLight)) +
         (x12c_postConstructed->x110c_layerOffsets.size() * sizeof(rstl::pair< int, int >)) +
         (sizeof(CAreaObjectList));
}

CGameArea::CPostConstructed::CPostConstructed()
: x0_collision(nullptr)
, x8_collisionSize(0)
, x5c_bspTree(nullptr)
, xa0_pvs(nullptr)
, xa4_pvsEntityMap(SPVSActorInfo(0xffff, kInvalidUniqueId))
, x10a8_pvsVersion(0)
, x10bc_pathArea(nullptr)
, x10c0_areaObjectList(nullptr)
, x10c4_areaFog(nullptr)
, x10c8_sclyBuf(nullptr)
, x10d0_sclySize(0)
, x10d4_firstMatPtr(nullptr)
, x10d8_areaAttributes(nullptr)
, x10dc_occlusionState(kOS_Occluded)
, x10e0_(0)
, x10e4_occludedTime(skEntityThinkDisableDelayOnOcclusion)
, x10e8_(-1)
, x10ec_firstMatSection(0)
, x1100_(0)
, x1104_(0)
, x1108_24_(false)
, x1108_25_modelsConstructed(false)
, x1108_26_(false)
, x1108_28_occlusionPinged(false)
, x1108_29_pvsHasActors(false)
, x1108_30_(false)
, x111c_thermalCurrent(0.f)
, x1120_thermalSpeed(0.f)
, x1124_thermalTarget(0.f)
, x1128_worldLightingLevel(1.f)
, x112c_xraySpeed(0.f)
, x1130_xrayTarget(1.f)
, x1134_weaponWorldLightingSpeed(0.f)
, x1138_weaponWorldLightingTarget(1.f)
, x113c_playerActorsLoading(0) {}

CGameArea::CGameArea(CInputStream& in, int idx, int mlvlVersion)
: x4_selfIdx(idx)
, x8_nameSTRG(in.ReadLong())
, xc_transform(in)
, x3c_invTransform(xc_transform.GetInverse())
, x6c_aabb(in)
, x84_mrea(in.ReadLong())
, x88_areaId(mlvlVersion > 15 ? in.ReadLong() : -1)
, x8c_attachedAreaIndices(in)
, x9c_deps1(in)
, xac_deps2(in)
, xec_totalResourcesSize(0)
, xf0_24_postConstructed(false)
, xf0_25_active(true)
, xf0_26_tokensReady(false)
, xf0_27_loadPaused(false)
, xf0_28_validated(false)
, xf4_phase(kP_LoadHeader)
, x120_unk(0)
, x124_secCount(0)
, x128_mreaDataOffset(0)
, x12c_postConstructed(nullptr)
, x130_next(nullptr)
, x134_prev(nullptr)
, x138_curChain(-1) {
  x6c_aabb = x6c_aabb.GetTransformedAABox(xc_transform);
  if (mlvlVersion > 13) {
    xbc_layerDepOffsets = rstl::vector< uint >(in);
  } else {
    xbc_layerDepOffsets.reserve(1);
    xbc_layerDepOffsets.push_back(0);
  }
  int dockCount = in.Get< int >();
  xcc_docks.reserve(dockCount);
  for (int i = 0; i < dockCount; ++i) {
    xcc_docks.push_back(Dock(in, xc_transform));
  }
  ClearTokenList();
  for (int i = 0; i < xdc_tokens.size(); ++i) {
    xec_totalResourcesSize +=
        gpResourceFactory->ResourceSize(SObjectTag(xac_deps2[i].second, xac_deps2[i].first));
  }
  xec_totalResourcesSize += gpResourceFactory->ResourceSize(SObjectTag('MREA', x84_mrea));
  CMemoryDrawEnum::AddWorldMemory(GetPreConstructedSize());
}

CGameArea::~CGameArea() {
  if (xf0_24_postConstructed) {
    CMemoryDrawEnum::SubtractWorldMemory(GetPostConstructedSize());
  }
  CMemoryDrawEnum::SubtractWorldMemory(GetPreConstructedSize());
  if (xf0_24_postConstructed) {
    RemoveStaticGeometry();
  } else {
    while (!Invalidate(nullptr)) {
    }
  }
}

void CGameArea::ClearTokenList() {
  if (xdc_tokens.capacity() == 0) {
    xdc_tokens.reserve(xac_deps2.size());
  } else {
    xdc_tokens.clear();
  }
  xf0_26_tokensReady = false;
}

void CGameArea::VerifyTokenList(CStateManager& mgr) {
  if (xdc_tokens.empty()) {
    ClearTokenList();
    const CScriptLayerManager& layers = *mgr.WorldLayerState();
    if (!xac_deps2.empty()) {
      for (int layer = xbc_layerDepOffsets.size() - 1; layer >= 0; --layer) {
        if (layers.IsLayerActive(x4_selfIdx, layer)) {
          int start = xbc_layerDepOffsets[layer];
          int end = layer + 1 < xbc_layerDepOffsets.size() ? xbc_layerDepOffsets[layer + 1]
                                                           : xac_deps2.size();
          for (int i = start; i < end; ++i) {
            const rstl::pair< uint, uint >& dep = xac_deps2[i];
            xdc_tokens.push_back(gpSimplePool->GetObj(SObjectTag(dep.second, dep.first)));
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
  if (x12c_postConstructed->x1108_25_modelsConstructed) {
    return;
  }
  int section = x12c_postConstructed->x10ec_firstMatSection;
  auto nextSection = [&]() -> std::span< uchar > {
    if (section < 0 || section >= x110_mreaSecBufs.size()) {
      Log.fatal("Missing MREA model section");
    }
    const auto& part = x110_mreaSecBufs[section++];
    if (part.second < 0 || (part.second && part.first.null())) {
      Log.fatal("Invalid MREA model section");
    }
    return part.second ? std::span(reinterpret_cast< uchar* >(part.first.get()),
                                   static_cast< size_t >(part.second))
                       : std::span< uchar >{};
  };
  const TModelData materials = nextSection();
  const uint materialCount = ValidateModelMaterials(materials);
  x12c_postConstructed->x10d4_firstMatPtr = materials.data();
  x12c_postConstructed->x4c_insts.clear();
  const int modelCount = x12c_postConstructed->x4c_insts.capacity();
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
    const uint surfaceCount = ReadModelSurfaceCount(surfaceInfo, x110_mreaSecBufs.size() - section);
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
      x12c_postConstructed->x4c_insts.push_back(
          CMetroidModelInstance(header, materials, arrays, surfaces));
    }
  }
#else
  AUTO(section, x110_mreaSecBufs.begin() + x12c_postConstructed->x10ec_firstMatSection);
  x12c_postConstructed->x10d4_firstMatPtr = reinterpret_cast< const u8* >(section->first.get());
  x12c_postConstructed->x4c_insts.clear();
  ++section;
  const int modelCount = x12c_postConstructed->x4c_insts.capacity();
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
      x12c_postConstructed->x4c_insts.push_back(
          CMetroidModelInstance(header, x12c_postConstructed->x10d4_firstMatPtr, positions, normals,
                                colors, texCoords, packedTexCoords, surfaces));
      surfaces.clear();
    }
  }
#endif
  x12c_postConstructed->x1108_25_modelsConstructed = true;
}

static inline CVector3f SwapVectorBytes(CVector3f vec) {
  return CVector3f(CBasics::SwapBytes(vec.GetX()), CBasics::SwapBytes(vec.GetY()),
                   CBasics::SwapBytes(vec.GetZ()));
}

void CGameArea::PostConstructArea() {
  const int version = VerifyHeader();
  rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > >::const_iterator section =
      x110_mreaSecBufs.begin();
  const SMREAHeader* header = reinterpret_cast< const SMREAHeader* >(section->first.get());
  // Retail retains these transform comparisons with their results discarded.
  for (int i = 0; i < 3; ++i) {
    CVector3f row = SwapVectorBytes(header->transform.GetRow(i));
    close_enough(xc_transform.GetRow(i), row, 0.001f);
  }
  CVector3f translation = SwapVectorBytes(header->transform.GetTranslation());
  close_enough(xc_transform.GetTranslation(), translation, 0.001f);

  const int modelCount = CBasics::SwapBytes(header->modelCount);
  section += 2;
  int firstGeometry = section - x110_mreaSecBufs.begin();
  x12c_postConstructed->x10ec_firstMatSection = firstGeometry;
  ++section;
  x12c_postConstructed->x4c_insts.reserve(modelCount);
  for (int i = 0; i < modelCount; ++i) {
#if TARGET_LITTLE_ENDIAN
    int surfaces = CBasics::SwapBytes(*reinterpret_cast< const int* >((section + 6)->first.get()));
#else
    int surfaces = *reinterpret_cast< const int* >((section + 6)->first.get());
#endif
    section += 7;
    section += surfaces;
  }

  long geometryEnd = section - x110_mreaSecBufs.begin();
  if (version >= 15 && CBasics::SwapBytes(header->renderOctreeSection) != -1) {
    rstl::auto_ptr< const u8 > buffer(reinterpret_cast< const u8* >(section->first.get()));
    buffer.release();
    x12c_postConstructed->xc_octTree = CAreaRenderOctTree(buffer);
    ++section;
  }

  x12c_postConstructed->x10c8_sclyBuf = section->first.get();
  x12c_postConstructed->x10c8_sclyBuf.release();
  x12c_postConstructed->x10d0_sclySize = section->second;
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
  x12c_postConstructed->x0_collision = collision;
  if (!collisionOwned) {
    x12c_postConstructed->x0_collision.release();
  }
  x12c_postConstructed->x8_collisionSize = collisionSize;
  ++section;
  {
    CMemoryInStream stream(section->first.get(), section->second);
    x12c_postConstructed->x5c_bspTree = rs_new CAreaBspTree(stream, xc_transform);
  }

  if (version > 6) {
    ++section;
    CMemoryInStream stream(section->first.get(), section->second);
    const uint magic = stream.ReadLong();
    const bool twoLayers = magic == 0xbabedead;
    int count = twoLayers ? stream.ReadLong() : magic;
    x12c_postConstructed->x60_lightsA.reserve(count);
    x12c_postConstructed->x70_gfxLightsA.reserve(count);
    for (int i = 0; i < count; ++i) {
      x12c_postConstructed->x60_lightsA.push_back(CWorldLight(stream));
      x12c_postConstructed->x70_gfxLightsA.push_back(
          x12c_postConstructed->x60_lightsA[i].GetAsCGraphicsLight());
    }
    if (twoLayers) {
      const int countB = stream.Get< int >();
      if (countB != 0) {
        x12c_postConstructed->x80_lightsB.reserve(countB);
        x12c_postConstructed->x90_gfxLightsB.reserve(countB);
        for (int i = 0; i < countB; ++i) {
          x12c_postConstructed->x80_lightsB.push_back(CWorldLight(stream));
          x12c_postConstructed->x90_gfxLightsB.push_back(
              x12c_postConstructed->x80_lightsB[i].GetAsCGraphicsLight());
        }
      }
    }

    const CPostConstructed* post = x12c_postConstructed.get();
    if (post->x80_lightsB.size() == 0) {
      x12c_postConstructed->x80_lightsB = x12c_postConstructed->x60_lightsA;
      x12c_postConstructed->x90_gfxLightsB = x12c_postConstructed->x70_gfxLightsA;
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
        x12c_postConstructed->x10a8_pvsVersion = pvsVersion;
        if (x12c_postConstructed->x10a8_pvsVersion == 2) {
          x12c_postConstructed->x1108_29_pvsHasActors = stream.ReadBool();
          x12c_postConstructed->x1108_30_ = stream.ReadBool();
          x12c_postConstructed->xa0_pvs =
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
      x12c_postConstructed->x10ac_pathToken =
          TLockedToken< CPFArea >(gpSimplePool->GetObj(SObjectTag('PATH', pathId)));
      x12c_postConstructed->x10bc_pathArea = **x12c_postConstructed->x10ac_pathToken;
      x12c_postConstructed->x10bc_pathArea->SetTransform(xc_transform);
    }
  }

#if defined(TARGET_PC)
  x12c_postConstructed->x1108_24_ = true;
#else
  int firstARAM = firstGeometry;
  int sectionCount = x110_mreaSecBufs.size();
  for (; firstARAM < sectionCount; ++firstARAM) {
    if (x110_mreaSecBufs[firstARAM].first.owner()) {
      break;
    }
  }
  int lastARAM = geometryEnd;
  for (; firstARAM < lastARAM; --lastARAM) {
    if (x110_mreaSecBufs[lastARAM].first.owner()) {
      break;
    }
  }
  if (firstARAM < lastARAM) {
    x12c_postConstructed->x10e8_ = firstARAM;
    int bufferCount = 0;
    for (int i = firstARAM; i < lastARAM; ++i) {
      if (x110_mreaSecBufs[i].first.owner()) {
        ++bufferCount;
      }
    }
    x12c_postConstructed->x10f0_tokens.reserve(bufferCount);
    for (int part = firstARAM; part < lastARAM;) {
      int start = part;
      uint size = x110_mreaSecBufs[part++].second;
      for (; part < lastARAM && !x110_mreaSecBufs[part].first.owner(); ++part) {
        size += x110_mreaSecBufs[part].second;
      }
      x12c_postConstructed->x10f0_tokens.push_back(rstl::pair< CARAMToken, int >(
          CARAMToken(x110_mreaSecBufs[start].first.release(), size, 1), part - start));
      x12c_postConstructed->x1104_ += size;
      if (GetOcclusionState() == kOS_Occluded) {
        CARAMToken& token = x12c_postConstructed->x10f0_tokens.back().first;
        token.LoadToARAM();
        if (token.GetStatus() != CARAMToken::kS_One) {
          x12c_postConstructed->x1100_ += size;
        }
      }
    }
    bool modelsInMRAM = GetOcclusionState() != kOS_Occluded;
    x12c_postConstructed->x1108_24_ = modelsInMRAM;
  }
#endif

  x12c_postConstructed->x10c0_areaObjectList = rs_new CAreaObjectList(x4_selfIdx);
  x12c_postConstructed->x10c4_areaFog = rs_new CAreaFog;
  xf0_24_postConstructed = true;
  CMemoryDrawEnum::AddWorldMemory(GetPostConstructedSize());

  CMemoryInStream stream(x12c_postConstructed->x10c8_sclyBuf.get(), GetScriptingSize());
  if (stream.ReadLong() == 'SCLY') {
    stream.ReadLong();
    int layerCount = stream.ReadLong();
    rstl::vector< int > sizes;
    sizes.reserve(layerCount);
    for (int i = 0; i < layerCount; ++i) {
      sizes.push_back(stream.Get< int >());
    }
    const int firstLayerOffset = (layerCount + 3) * 4;
    x12c_postConstructed->x110c_layerOffsets.reserve(layerCount);
    for (int i = 0; i < layerCount; ++i) {
      int offset = i == 0 ? firstLayerOffset
                          : x12c_postConstructed->x110c_layerOffsets[i - 1].first +
                                x12c_postConstructed->x110c_layerOffsets[i - 1].second;
      x12c_postConstructed->x110c_layerOffsets.push_back(rstl::pair< int, int >(offset, sizes[i]));
    }
  } else {
    x12c_postConstructed->x110c_layerOffsets.reserve(1);
    x12c_postConstructed->x110c_layerOffsets.push_back(
        rstl::pair< int, int >(0, GetScriptingSize()));
  }
}

void CGameArea::Validate(CStateManager& mgr) {
  if (!xf0_24_postConstructed) {
    while (StartStreamingMainArea()) {
    }
    for (AUTO(it, xf8_loadTransactions.begin()); it != xf8_loadTransactions.end(); ++it) {
      if (it->get() != nullptr) {
        (*it)->WaitUntilComplete();
      }
    }
    if (xdc_tokens.empty()) {
      VerifyTokenList(mgr);
      for (AUTO(it, xdc_tokens.begin()); it != xdc_tokens.end(); ++it) {
        it->Lock();
      }
      for (AUTO(it, xdc_tokens.begin()); it != xdc_tokens.end(); ++it) {
        it->GetObj();
      }
      xf0_26_tokensReady = true;
    }
    xf8_loadTransactions.clear();
    xf4_phase = kP_LoadHeader;
    PostConstructArea();
    if (x4_selfIdx != kInvalidAreaId) {
      mgr.World()->MoveAreaToChain3(x4_selfIdx);
    }
    LoadScriptObjects(mgr);
    if (x12c_postConstructed->xa0_pvs.get() != nullptr &&
        x12c_postConstructed->x1108_29_pvsHasActors) {
      for (int i = 0; i < x12c_postConstructed->xa0_pvs->GetNumActors(); ++i) {
        const CPostConstructed* post = x12c_postConstructed.get();
        uint editorId = post->xa0_pvs->GetEntityIdByIndex(i) | (x4_selfIdx.Value() << 16);
        TUniqueId id = mgr.GetIdForScript(editorId);
        if (id != kInvalidUniqueId) {
          const CPVSAreaSet* pvs = x12c_postConstructed->xa0_pvs.get();
          x12c_postConstructed->xa4_pvsEntityMap[id.Value()] =
              SPVSActorInfo(i + (pvs->GetNumFeatures() - pvs->GetNumActors()), id);
        }
      }
    }
    xf0_28_validated = true;
    mgr.AreaLoaded(GetId());
  }
}

void CGameArea::LoadScriptObjects(CStateManager& mgr) {
  rstl::vector< TEditorId > ids;
  const CScriptLayerManager& layers = *mgr.WorldLayerState();
  int count = layers.GetAreaLayerCount(x4_selfIdx);
  for (int i = 0; i < count; ++i) {
    TLayerId layer(i);
    if (layers.IsLayerActive(x4_selfIdx, layer)) {
      rstl::pair< const uchar*, int > buffer = GetLayerScriptBuffer(layer);
      CMemoryInStream stream(buffer.first, buffer.second);
      mgr.LoadScriptObjects(GetId(), stream, ids);
    }
  }
  mgr.InitScriptObjects(ids);
}

void CGameArea::StartStreamIn(CStateManager& mgr) {
  bool fullyInitialized = mgr.IsFullyInitialized();
  if (!xf0_24_postConstructed && !xf0_27_loadPaused) {
    VerifyTokenList(mgr);
    if (!xf0_26_tokensReady) {
      int notLoaded = 0;
      for (int i = 0; i < xdc_tokens.size(); ++i) {
        CToken& token = xdc_tokens[i];
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
          gpResourceFactory->GetResLoader().FindResource(SObjectTag('MREA', x84_mrea));
          token.Lock();
          ++notLoaded;
        }
      }
      if (notLoaded == 0) {
        xf0_26_tokensReady = true;
      } else {
        return;
      }
    }
    StartStreamingMainArea();
    if (xf4_phase == kP_WaitForFinish) {
      CullDeadAreaRequests();
      if (xf8_loadTransactions.empty()) {
        Validate(mgr);
      }
    }
  }
}

void CGameArea::CullDeadAreaRequests() {
  while (!xf8_loadTransactions.empty() && xf8_loadTransactions.front()->IsComplete()) {
    xf8_loadTransactions.pop_front();
  }
}

void CGameArea::KillmAreaData() {
  x110_mreaSecBufs = rstl::vector< rstl::pair< rstl::auto_ptr< char >, int > >();
}

bool CGameArea::Invalidate(CStateManager* mgr) {
  if (!xf0_24_postConstructed) {
    ClearTokenList();
    for (AUTO(it, xf8_loadTransactions.begin()); it != xf8_loadTransactions.end();) {
      AUTO(cur, it);
      ++it;
      if (!(*cur)->IsComplete()) {
        (*cur)->PostCancelRequest();
      } else {
        xf8_loadTransactions.erase(cur);
      }
    }
    if (!xf8_loadTransactions.empty()) {
      return false;
    }
    x12c_postConstructed = nullptr;
    xf4_phase = kP_LoadHeader;
    KillmAreaData();
    return true;
  }
  if (mgr != nullptr) {
    mgr->PrepareAreaUnload(GetId());
  }
  CMemoryDrawEnum::SubtractWorldMemory(GetPostConstructedSize());
  RemoveStaticGeometry();
  x12c_postConstructed = nullptr;
  xf0_24_postConstructed = false;
  xf0_28_validated = false;
  xf4_phase = kP_LoadHeader;
  CullDeadAreaRequests();
  KillmAreaData();
  x120_unk = 0;
  ClearTokenList();
  if (mgr != nullptr) {
    mgr->AreaUnloaded(GetId());
  }
  return true;
}

char* CGameArea::AllocNewAreaData(int offset, int size) {
  char* buffer = static_cast< char* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  rstl::pair< rstl::auto_ptr< char >, int > data(buffer, size);
  x110_mreaSecBufs.push_back(data);
  SObjectTag tag('MREA', x84_mrea);
  xf8_loadTransactions.push_back(rstl::auto_ptr< CDvdRequest >(
      gpResourceFactory->GetResLoader().LoadResourcePartAsync(tag, offset, size, buffer)));
  return buffer;
}

int CGameArea::GetNumPartSizes() const {
  return CBasics::SwapBytes(
      reinterpret_cast< const int* >(x110_mreaSecBufs.front().first.get())[15]);
}

bool CGameArea::ReloadAllUnloadedTextures() {
  bool finished = true;
  for (int i = 0; i < xdc_tokens.size(); ++i) {
    CToken& token = xdc_tokens[i];
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
  for (int i = 0; i < xdc_tokens.size(); ++i) {
    CToken& token = xdc_tokens[i];
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
  if (xf0_24_postConstructed) {
    return false;
  }
  switch (xf4_phase) {
  case kP_LoadHeader: {
    x110_mreaSecBufs.reserve(2);
    AllocNewAreaData(0, 96);
    x12c_postConstructed = rs_new CPostConstructed();
    xf4_phase = kP_LoadSecSizes;
    break;
  }
  case kP_LoadSecSizes: {
    CullDeadAreaRequests();
    if (xf8_loadTransactions.empty()) {
      VerifyHeader();
      int count = GetNumPartSizes();
      AllocNewAreaData(x110_mreaSecBufs[0].second, ROUND_UP_32(count * 4));
      xf4_phase = kP_ReserveSections;
    }
    break;
  }
  case kP_ReserveSections: {
    CullDeadAreaRequests();
    if (xf8_loadTransactions.empty()) {
#if TARGET_LITTLE_ENDIAN
      // Decode the loaded size table once, before any section offsets are calculated.
      int* sizes = reinterpret_cast< int* >(x110_mreaSecBufs[1].first.get());
      for (int i = 0; i < GetNumPartSizes(); ++i) {
        sizes[i] = CBasics::SwapBytes(sizes[i]);
      }
#endif
      x110_mreaSecBufs.reserve(GetNumPartSizes() + 2);
      int headerSize = x110_mreaSecBufs[0].second;
      int sizesSize = x110_mreaSecBufs[1].second;
      x124_secCount = 0;
      headerSize += sizesSize;
      x128_mreaDataOffset = headerSize;
      xf4_phase = kP_LoadDataSections;
    }
    break;
  }
  case kP_LoadDataSections: {
    CullDeadAreaRequests();
    int secCount = x124_secCount;
    int totalSize = 0;
    int partSizes = GetNumPartSizes();
    const int* sizes = reinterpret_cast< const int* >(x110_mreaSecBufs[1].first.get());
    SObjectTag tag('MREA', x84_mrea);
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
    xf8_loadTransactions.push_back(
        rstl::auto_ptr< CDvdRequest >(gpResourceFactory->GetResLoader().LoadResourcePartAsync(
            tag, x128_mreaDataOffset, totalSize, buffer.get())));
    x128_mreaDataOffset += totalSize;
    const int firstSize = sizes[secCount];
    int offset = firstSize;
    x110_mreaSecBufs.push_back(rstl::pair< rstl::auto_ptr< char >, int >(buffer, firstSize));
    for (int i = secCount + 1; i < targetSecCount; ++i) {
      rstl::auto_ptr< char > section(buffer.get() + offset);
      section.release();
      int size = sizes[i];
      x110_mreaSecBufs.push_back(rstl::pair< rstl::auto_ptr< char >, int >(section, size));
      offset += size;
    }
    x124_secCount = targetSecCount;
    if (targetSecCount == partSizes) {
      x120_unk = x128_mreaDataOffset;
      xf4_phase = kP_WaitForFinish;
    }
    break;
  }
  case kP_WaitForFinish: {
    CullDeadAreaRequests();
    if (xf8_loadTransactions.empty()) {
      return false;
    }
    break;
  }
  }
  return true;
}

int CGameArea::SetChain(CGameArea* next, int chain) {
  if (x138_curChain == chain) {
    return x138_curChain;
  }
  if (x134_prev != nullptr) {
    x134_prev->x130_next = x130_next;
  }
  if (x130_next != nullptr) {
    x130_next->x134_prev = x134_prev;
  }
  x134_prev = nullptr;
  x130_next = next;
  if (next != nullptr) {
    next->x134_prev = this;
  }
  int oldChain = x138_curChain;
  x138_curChain = chain;
  return oldChain;
}

bool CGameArea::TransferARAMTokensOver(EARAMTransfer mode) {
#if defined(TARGET_PC)
  return true;
#else
  if (x12c_postConstructed->x1108_24_) {
    return true;
  }
  bool finished = true;
  int part = x12c_postConstructed->x10e8_;
  for (AUTO(it, x12c_postConstructed->x10f0_tokens.begin());
       it != x12c_postConstructed->x10f0_tokens.end(); ++it) {
    if (it->first.GetStatus() != CARAMToken::kS_One) {
      x12c_postConstructed->x1100_ -= it->first.GetSize();
    }
    if (mode == kAT_Async && !it->first.LoadToMRAM()) {
      finished = false;
    } else if (finished) {
      char* buffer = static_cast< char* >(it->first.GetMRAMSafe());
      int offset = 0;
      for (int i = 0; i < it->second; ++i) {
        rstl::auto_ptr< char > section(buffer + offset);
        section.release();
        offset += x110_mreaSecBufs[part].second;
        x110_mreaSecBufs[part].first = section;
        ++part;
      }
    }
  }
  x12c_postConstructed->x1108_24_ = finished;
  return finished;
#endif
}

bool CGameArea::TransferTokensToARAM() {
#if defined(TARGET_PC)
  return true;
#else
  bool finished = true;
  int part = x12c_postConstructed->x10e8_;
  AUTO(it, x12c_postConstructed->x10f0_tokens.begin());
  rstl::auto_ptr< char > empty;
  for (; it != x12c_postConstructed->x10f0_tokens.end(); ++it) {
    for (int i = 0; i < it->second; ++i) {
      x110_mreaSecBufs[part].first = empty;
      ++part;
    }
    CARAMToken::EStatus oldStatus = it->first.GetStatus();
    it->first.LoadToARAM();
    if (oldStatus == CARAMToken::kS_One && it->first.GetStatus() != CARAMToken::kS_One) {
      x12c_postConstructed->x1100_ += it->first.GetSize();
    }
    if (it->first.GetStatus() >= CARAMToken::kS_Two &&
        it->first.GetStatus() <= CARAMToken::kS_Five) {
      finished = false;
    }
  }
  x12c_postConstructed->x1108_24_ = false;
  x12c_postConstructed->x1108_25_modelsConstructed = false;
  return finished;
#endif
}

void CGameArea::AddStaticGeometry() {
  if (x12c_postConstructed->x10dc_occlusionState != kOS_Visible) {
    x12c_postConstructed->x10e0_ = 0;
    x12c_postConstructed->x10dc_occlusionState = kOS_Visible;
    TransferARAMTokensOver(kAT_Blocking);
    if (!x12c_postConstructed->x1108_25_modelsConstructed) {
      FillInStaticGeometry();
    }
    const CPostConstructed* post = x12c_postConstructed.get();
    int areaIdx = x4_selfIdx.Value();
    const CAreaRenderOctTree* tree =
        post->xc_octTree.valid() ? post->xc_octTree.get_ptr() : nullptr;
    gpRender->AddStaticGeometry(&x12c_postConstructed->x4c_insts, tree, areaIdx);
  }
}

void CGameArea::RemoveStaticGeometry() {
  if (xf0_24_postConstructed && x12c_postConstructed.get() != nullptr &&
      x12c_postConstructed->x10dc_occlusionState != kOS_Occluded) {
    x12c_postConstructed->x10e0_ = 0;
    x12c_postConstructed->x10dc_occlusionState = kOS_Occluded;
    gpRender->RemoveStaticGeometry(&x12c_postConstructed->x4c_insts);
  }
}

void CGameArea::SetOcclusionState(EOcclusionState state) {
  if (xf0_24_postConstructed && state != x12c_postConstructed->x10dc_occlusionState) {
    if (state == kOS_Occluded) {
      x12c_postConstructed->x1108_26_ = true;
      x12c_postConstructed->x1108_27_ = false;
      RemoveStaticGeometry();
    } else {
      ReloadAllUnloadedTextures();
      AddStaticGeometry();
    }
  }
}

void CGameArea::AliveUpdate(float dt) {
  if (x12c_postConstructed->x10dc_occlusionState == kOS_Occluded) {
    x12c_postConstructed->x10e4_occludedTime += dt;
  } else {
    x12c_postConstructed->x10e4_occludedTime = 0.f;
  }
  UpdateFog(dt);
  UpdateThermalVisor(dt);
  UpdateWeaponWorldLighting(dt);
}

void CGameArea::PreRender() {
  if (xf0_24_postConstructed) {
    if (x12c_postConstructed->x1108_28_occlusionPinged) {
      x12c_postConstructed->x1108_28_occlusionPinged = false;
    } else {
      PingOcclusionState();
    }
  }
}

void CGameArea::PingOcclusionState() {
  if (x12c_postConstructed->x10dc_occlusionState == kOS_Occluded) {
    if (x12c_postConstructed->x10e0_ < 2) {
      ++x12c_postConstructed->x10e0_;
      return;
    }
    x12c_postConstructed->x10e0_ = 3;
    if (!x12c_postConstructed->x1108_27_) {
      bool unloaded = UnloadAllloadedTextures();
      bool transferred = TransferTokensToARAM();
      if (unloaded && transferred) {
        x12c_postConstructed->x1108_27_ = true;
      }
    }
  }
  x12c_postConstructed->x1108_26_ = true;
}

void CGameArea::OtherAreaOcclusionChanged() {
  if (x12c_postConstructed->x10e0_ == 3 &&
      x12c_postConstructed->x10dc_occlusionState == kOS_Occluded) {
    bool unloaded = UnloadAllloadedTextures();
    bool transferred = TransferTokensToARAM();
    x12c_postConstructed->x1108_27_ = unloaded && transferred;
    return;
  }
  if (x12c_postConstructed->x10dc_occlusionState == kOS_Visible) {
    ReloadAllUnloadedTextures();
  }
}

bool IGameArea::Dock::IsReferenced() const { return x48_isReferenced; }

int IGameArea::Dock::GetReferenceCount() const { return x0_referenceCount; }

void IGameArea::Dock::SetReferenceCount(int count) {
  x0_referenceCount = count;
  x48_isReferenced = true;
}

TAreaId IGameArea::Dock::GetConnectedAreaId(int other) const {
  return x4_dockReferences.empty() ? TAreaId(-1) : x4_dockReferences[other].x0_area;
}

int IGameArea::Dock::GetOtherDockNumber(int other) const {
  if (x4_dockReferences.empty()) {
    return -1;
  }
  return x4_dockReferences[other].x4_dock;
}

bool IGameArea::Dock::ShouldLoadOtherArea(int other) const {
  if (x4_dockReferences.empty()) {
    return false;
  }
  return x4_dockReferences[other].x6_loadOther;
}

void IGameArea::Dock::SetShouldLoadOther(int other, bool should) {
  if (other < x4_dockReferences.size()) {
    x4_dockReferences[other].x6_loadOther = should;
  }
}

bool IGameArea::Dock::GetShouldLoadOther(int other) const {
  if (other < x4_dockReferences.size()) {
    return x4_dockReferences[other].x6_loadOther;
  }
  return false;
}

uchar CGameArea::CAreaObjectList::IsQualified(const CEntity& ent) {
  return x200c_areaId == ent.GetCurrentAreaId();
}

CGameArea::CAreaFog::CAreaFog()
: x0_fogMode(kRFM_None)
, x4_rangeCur(0.f, 1024.f)
, xc_rangeTarget(x4_rangeCur)
, x14_rangeDelta(0.f, 0.f)
, x1c_colorCur(0.5f, 0.5f, 0.5f)
, x28_colorTarget(x1c_colorCur)
, x34_colorDelta(0.f) {}

void CGameArea::CAreaFog::DisableFog() { x0_fogMode = kRFM_None; }

bool CGameArea::CAreaFog::IsFogDisabled() const { return x0_fogMode == kRFM_None; }

void CGameArea::CAreaFog::SetFogExplicit(ERglFogMode mode, const CColor& color,
                                         const CVector2f& range) {
  x0_fogMode = mode;
  x1c_colorCur = x28_colorTarget = color.ToVector3f();
  x4_rangeCur = xc_rangeTarget = range;
}

void CGameArea::CAreaFog::FadeFog(const ERglFogMode mode, const CColor& color,
                                  const CVector2f& vec1, const float speed, const CVector2f& vec2) {
  if (x0_fogMode == kRFM_None) {
    x0_fogMode = mode;
    x1c_colorCur = x28_colorTarget = color.ToVector3f();
    x4_rangeCur = CVector2f(vec1.GetY(), vec1.GetY());
    xc_rangeTarget = vec1;
  } else {
    x0_fogMode = mode;
    x28_colorTarget = color.ToVector3f();
    xc_rangeTarget = vec1;
  }

  x34_colorDelta = speed;
  x14_rangeDelta = vec2;
}

void CGameArea::CAreaFog::RollFogOut(const float rangeDelta, const float colorDelta,
                                     const CColor& color) {
  x14_rangeDelta = CVector2f(rangeDelta, rangeDelta * 2.f);
  xc_rangeTarget = CVector2f(4096.f, 4096.f);
  x34_colorDelta = colorDelta;
  x28_colorTarget = color.ToVector3f();
}

void CGameArea::UpdateFog(const float dt) {
  if (!x12c_postConstructed->x10c4_areaFog.null()) {
    x12c_postConstructed->x10c4_areaFog->Update(dt);
  }
}

void CGameArea::CAreaFog::Update(const float dt) {
  if (x0_fogMode == kRFM_None) {
    return;
  }

  if (!(x34_colorDelta > 0.f) && x14_rangeDelta == CVector2f(0.f, 0.f)) {
    return;
  }

  float current[5];
  float target[5];
  float result[5];
  float step[5] = {0.f, 0.f, 0.f, 0.f, 0.f};
  step[2] = step[1] = step[0] = x34_colorDelta * dt;
  step[3] = dt * x14_rangeDelta.GetX();
  step[4] = dt * x14_rangeDelta.GetY();
  current[0] = x1c_colorCur.GetX();
  target[0] = x28_colorTarget.GetX();
  current[1] = x1c_colorCur.GetY();
  target[1] = x28_colorTarget.GetY();
  current[2] = x1c_colorCur.GetZ();
  target[2] = x28_colorTarget.GetZ();
  current[3] = x4_rangeCur.GetX();
  current[4] = x4_rangeCur.GetY();
  target[3] = xc_rangeTarget.GetX();
  target[4] = xc_rangeTarget.GetY();
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
    x34_colorDelta = 0.f;
    x14_rangeDelta = CVector2f(0.f, 0.f);
    if (result[3] == result[4]) {
      x0_fogMode = kRFM_None;
    }
  }
  if (result[3] > result[4]) {
    result[3] = result[4];
  }
  x1c_colorCur = CVector3f(result[0], result[1], result[2]);
  x4_rangeCur = CVector2f(result[3], result[4]);
}

void CGameArea::CAreaFog::SetCurrent() const {
  gpRender->SetWorldFog(x0_fogMode, x4_rangeCur.GetX(), x4_rangeCur.GetY(),
                        CColor(x1c_colorCur.GetX(), x1c_colorCur.GetY(), x1c_colorCur.GetZ(), 1.f));
}

bool CGameArea::DoesAreaNeedSkyNow() const {
  if (x12c_postConstructed.get() == nullptr) {
    return false;
  }
  if (x12c_postConstructed->x10d8_areaAttributes != nullptr) {
    return x12c_postConstructed->x10d8_areaAttributes->GetNeedsSky();
  }
  return false;
}

EEnvFxType CGameArea::DoesAreaNeedEnvFx() const {
  if (x12c_postConstructed.get() == nullptr) {
    return kEFX_None;
  }
  if (x12c_postConstructed->x10d8_areaAttributes == nullptr) {
    return kEFX_None;
  }
  if (x12c_postConstructed->x10dc_occlusionState != kOS_Visible) {
    return kEFX_None;
  }
  return x12c_postConstructed->x10d8_areaAttributes->GetEnvFxType();
}

bool CGameArea::TryTakingOutOfARAM() {
  if (x12c_postConstructed->x10dc_occlusionState == kOS_Occluded) {
    x12c_postConstructed->x1108_28_occlusionPinged = true;
  }
  return TransferARAMTokensOver(kAT_Async) && ReloadAllUnloadedTextures();
}

const CTransform4f& CGameArea::IGetTM() const { return xc_transform; }

CAssetId CGameArea::IGetStringTableAssetId() const { return x8_nameSTRG; }

uint CGameArea::IGetNumAttachedAreas() const { return x8c_attachedAreaIndices.size(); }

TAreaId CGameArea::IGetAttachedAreaId(int i) const { return x8c_attachedAreaIndices[i]; }

bool CGameArea::IIsActive() const { return xf0_25_active; }

CAssetId CGameArea::IGetAreaAssetId() const { return x84_mrea; }

int CGameArea::IGetAreaSaveId() const { return x88_areaId; }

rstl::pair< rstl::auto_ptr< char >, int > CGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

IGameArea::Dock::Dock(CInputStream& in, const CTransform4f& xf)
: x0_referenceCount(0), x48_isReferenced(false) {
  int count = in.Get< int >();
  x4_dockReferences.reserve(count);
  for (int i = 0; i < count; ++i) {
    TAreaId areaId(in.ReadLong());
    short dock = in.ReadLong();
    x4_dockReferences.push_back(SDockReference(areaId, dock, 0));
  }
  int vertCount = in.Get< int >();
  for (int i = 0; i < vertCount; ++i) {
    x14_planeVertices.push_back(xf * CVector3f(in));
  }
}

CDummyGameArea::CDummyGameArea(CInputStream& in, int idx, int mlvlVersion)
: x4_selfIdx(idx), x8_nameSTRG(-1), x14_transform(CTransform4f::Identity()) {
  x8_nameSTRG = in.ReadLong();
  x14_transform = CTransform4f(in);
  CAABox bounds(in);
  xc_mrea = in.ReadLong();
  if (mlvlVersion > 15) {
    x10_areaId = in.ReadLong();
  }
  x44_attachedAreaIndices = rstl::vector< ushort >(in);
  {
    rstl::vector< rstl::pair< uint, uint > > deps1(in);
    rstl::vector< rstl::pair< uint, uint > > deps2(in);
  }
  if (mlvlVersion > 13) {
    rstl::vector< uint > layerDepOffsets(in);
  }
  int count = in.Get< int >();
  x54_docks.reserve(count);
  for (int i = 0; i < count; ++i) {
    x54_docks.push_back(Dock(in, x14_transform));
  }
}

const CTransform4f& CDummyGameArea::IGetTM() const { return x14_transform; }

CAssetId CDummyGameArea::IGetStringTableAssetId() const { return x8_nameSTRG; }

uint CDummyGameArea::IGetNumAttachedAreas() const { return x44_attachedAreaIndices.size(); }

TAreaId CDummyGameArea::IGetAttachedAreaId(int idx) const { return x44_attachedAreaIndices[idx]; }

bool CDummyGameArea::IIsActive() const { return true; }

CAssetId CDummyGameArea::IGetAreaAssetId() const { return xc_mrea; }

int CDummyGameArea::IGetAreaSaveId() const { return x10_areaId; }

rstl::pair< rstl::auto_ptr< char >, int > CDummyGameArea::IGetScriptingMemoryAlways() const {
  return GetScriptingMemoryAlways(*this);
}

bool CGameArea::IsFinishedOccluding() const {
  if (x12c_postConstructed->x10dc_occlusionState == kOS_Occluded) {
    return x12c_postConstructed->x1108_27_;
  }
  return true;
}

rstl::pair< const uchar*, int > CGameArea::GetLayerScriptBuffer(const TLayerId layer) const {
  if (xf0_24_postConstructed) {
    const rstl::pair< int, int >& offsets = x12c_postConstructed->x110c_layerOffsets[layer.Value()];
    return rstl::pair< const uchar*, int >(
        reinterpret_cast< const uchar* >(x12c_postConstructed->x10c8_sclyBuf.get()) + offsets.first,
        offsets.second);
  }
  return rstl::pair< const uchar*, int >(nullptr, 0);
}

void CGameArea::SetLoadPauseState(bool paused) {
  if (!xf0_26_tokensReady) {
    xf0_27_loadPaused = paused;
    if (paused) {
      for (int i = 0; i < xdc_tokens.size(); ++i) {
        CToken& token = xdc_tokens[i];
        if (!token.IsLoaded()) {
          token.Unlock();
        }
      }
    }
  }
}

float CGameArea::GetXRayFogDistance() {
  if (x12c_postConstructed->x10d8_areaAttributes) {
    return x12c_postConstructed->x10d8_areaAttributes->GetXRayFogDistance();
  }
  return 1.f;
}

void CGameArea::SetAreaAttributes(CScriptAreaAttributes* attributes) {
  x12c_postConstructed->x10d8_areaAttributes = attributes;
  if (attributes) {
    x12c_postConstructed->x111c_thermalCurrent = attributes->GetThermalHeat();
    x12c_postConstructed->x1128_worldLightingLevel = attributes->GetWorldLightingLevel();
  }
}

void CGameArea::SetThermalSpeedAndTarget(float speed, float target) {
  x12c_postConstructed->x1120_thermalSpeed = speed;
  x12c_postConstructed->x1124_thermalTarget = target;
}

void CGameArea::SetXRaySpeedAndTarget(float speed, float target) {
  x12c_postConstructed->x112c_xraySpeed = speed;
  x12c_postConstructed->x1130_xrayTarget = target;
}

void CGameArea::SetWeaponWorldLighting(float speed, float target) {
  x12c_postConstructed->x1134_weaponWorldLightingSpeed = speed;
  x12c_postConstructed->x1138_weaponWorldLightingTarget = target;
}

void CGameArea::UpdateThermalVisor(float dt) {
  if (x12c_postConstructed->x1120_thermalSpeed == 0.f) {
    return;
  }
  float current = x12c_postConstructed->x111c_thermalCurrent;
  float delta = dt * x12c_postConstructed->x1120_thermalSpeed;
  if (CMath::AbsF(x12c_postConstructed->x1124_thermalTarget - current) < delta) {
    current = x12c_postConstructed->x1124_thermalTarget;
    x12c_postConstructed->x1120_thermalSpeed = 0.f;
  } else if (x12c_postConstructed->x1124_thermalTarget < current) {
    current -= delta;
  } else {
    current += delta;
  }
  x12c_postConstructed->x111c_thermalCurrent = current;
}

void CGameArea::UpdateWeaponWorldLighting(float dt) {
  float lighting = x12c_postConstructed->x1128_worldLightingLevel;
  if (0.f != x12c_postConstructed->x112c_xraySpeed) {
    float delta = dt * x12c_postConstructed->x112c_xraySpeed;
    if (CMath::AbsF(x12c_postConstructed->x1130_xrayTarget - lighting) < delta) {
      lighting = x12c_postConstructed->x1130_xrayTarget;
      x12c_postConstructed->x1134_weaponWorldLightingSpeed = 0.f;
    } else if (x12c_postConstructed->x1130_xrayTarget < lighting) {
      lighting -= delta;
    } else {
      lighting += delta;
    }
  }
  if (0.f != x12c_postConstructed->x1134_weaponWorldLightingSpeed) {
    float weaponLighting = x12c_postConstructed->x1128_worldLightingLevel;
    float delta = dt * x12c_postConstructed->x1134_weaponWorldLightingSpeed;
    if (CMath::AbsF(x12c_postConstructed->x1138_weaponWorldLightingTarget - lighting) < delta) {
      weaponLighting = x12c_postConstructed->x1138_weaponWorldLightingTarget;
      x12c_postConstructed->x1134_weaponWorldLightingSpeed = 0.f;
    } else if (x12c_postConstructed->x1138_weaponWorldLightingTarget < weaponLighting) {
      weaponLighting -= delta;
    } else {
      weaponLighting += delta;
    }
    if (x12c_postConstructed->x112c_xraySpeed != 0.f) {
      lighting = rstl::min_val(weaponLighting, lighting);
    } else {
      lighting = weaponLighting;
    }
  }
  float epsilon = 0.00001f;
  if (!(fabs(x12c_postConstructed->x1128_worldLightingLevel - lighting) < epsilon)) {
    x12c_postConstructed->x1128_worldLightingLevel = lighting;
    CObjectList& objects = *x12c_postConstructed->x10c0_areaObjectList;
    for (int i = objects.GetFirstObjectIndex(); i != -1; i = objects.GetNextObjectIndex(i)) {
      if (CActor* actor = TCastToPtr< CActor >(objects[i])) {
        actor->SetWorldLightingDirty(true);
      }
    }
  }
}

uint CGameArea::LookupPVSID(TUniqueId id) {
  return x12c_postConstructed->xa4_pvsEntityMap[id.Value()].x0_pvsId;
}

TUniqueId CGameArea::LookupPVSUniqueID(TUniqueId id) {
  return x12c_postConstructed->xa4_pvsEntityMap[id.Value()].x2_uniqueId;
}

uint CGameArea::Get1stPVSLightFeature(uint light) const {
  const CPVSAreaSet* pvs = x12c_postConstructed->xa0_pvs.get();
  return !pvs ? -1 : pvs->GetNumFeatures() + pvs->GetNum2ndLights() + light;
}

uint CGameArea::Get2ndPVSLightFeature(uint light) const {
  const CPVSAreaSet* pvs = x12c_postConstructed->xa0_pvs.get();
  if (!x12c_postConstructed->x1108_30_ || !pvs) {
    return -1;
  }
  const int& count = pvs->GetNumFeatures();
  return count + light;
}
