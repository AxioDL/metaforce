#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/CSimplePool.hpp"

//TODO Remove WIP once we've transitioned to this
namespace metaforce::WIP {
#pragma region CModel
u32 CModel::sTotalMemory = 0;
u32 CModel::sFrameCounter = 0;
bool CModel::sIsTextureTimeoutEnabled = true;
CModel* CModel::sThisFrameList = nullptr;
CModel* CModel::sOneFrameList = nullptr;
CModel* CModel::sTwoFrameList = nullptr;

static u8* MemoryFromPartData(u8*& dataCur, const u32*& secSizeCur) {
  u8* ret = nullptr;
  if (*secSizeCur != 0) {
    ret = dataCur;
  }
  dataCur += hecl::SBig(*secSizeCur);
  ++secSizeCur;
  return ret;
}

CModel::CModel(std::unique_ptr<u8[]> in, u32 dataLen, IObjectStore* store)
: x0_data(std::move(in))
, x4_dataLen(dataLen)
, x34_next(sThisFrameList)
, x38_lastFrame(CGraphics::GetFrameCounter() - 2) {
  u8* data = x0_data.get();
  u32 flags = hecl::SBig(*reinterpret_cast<u32*>(data + 8));
  u32 sectionSizeStart = 0x2c;
  if (hecl::SBig(*reinterpret_cast<u32*>(data + 4)) == 1) {
    sectionSizeStart = 0x28;
  }
  const u32* secSizeCur = reinterpret_cast<u32*>(data + sectionSizeStart);
  s32 numMatSets = 1;
  if (hecl::SBig(*reinterpret_cast<u32*>(data + 4)) > 1) {
    numMatSets = hecl::SBig(*reinterpret_cast<s32*>(data + 0x28));
  }
  u8* dataCur = data + ROUND_UP_32(sectionSizeStart + hecl::SBig(*reinterpret_cast<s32*>(data + 0x24)) * 4);
  x18_matSets.reserve(numMatSets);
  for (s32 i = 0; i < numMatSets; ++i) {
    x18_matSets.emplace_back(static_cast<u8*>(MemoryFromPartData(dataCur, secSizeCur)));
    auto shader = x18_matSets.back();
    CCubeModel::MakeTexturesFromMats(shader.x10_data, shader.x0_textures, true);
    x4_dataLen += shader.x0_textures.size() * sizeof(TCachedToken<CTexture>);
  }

  /* Metaforce note: Due to padding in zeus types we need to convert these and store locally */
  u32 numVertices = hecl::SBig(*secSizeCur) / (sizeof(float) * 3);
  const u8* positions = MemoryFromPartData(dataCur, secSizeCur);
  for (u32 i = 0; i < numVertices; ++i) {
    const auto* pos = reinterpret_cast<const float*>(positions + (i * (sizeof(float) * 3)));
    m_positions.emplace_back(hecl::SBig(pos[0]), hecl::SBig(pos[1]), hecl::SBig(pos[2]));
  }
  u32 numNormals = hecl::SBig(*secSizeCur);
  numNormals /= ((flags & 2) == 0 ? sizeof(float) : sizeof(s16)) * 3;
  const u8* normals = MemoryFromPartData(dataCur, secSizeCur);

  for (u32 i = 0; i < numNormals; ++i) {
    if ((flags & 2) == 0) {
      const auto* norm = reinterpret_cast<const float*>(normals + (i * (sizeof(float) * 3)));
      m_floatNormals.emplace_back(hecl::SBig(norm[0]), hecl::SBig(norm[1]), hecl::SBig(norm[2]));
    } else {
      const auto* norm = reinterpret_cast<const s16*>(normals + (i * (sizeof(s16) * 3)));
      m_shortNormals.emplace_back(std::array{hecl::SBig(norm[0]), hecl::SBig(norm[1]), hecl::SBig(norm[2])});
    }
  }
  u32 numColors = hecl::SBig(*secSizeCur) / (sizeof(int));
  const u8* vtxColors = MemoryFromPartData(dataCur, secSizeCur);

  for (u32 i = 0; i < numColors; ++i) {
    const u32 col = hecl::SBig(*reinterpret_cast<const u32*>(vtxColors + (i * (sizeof(u32)))));
    m_colors.emplace_back(zeus::CColor(zeus::Comp32(col)));
  }

  u32 numFloatUVs = hecl::SBig(*reinterpret_cast<const u32*>(secSizeCur)) / (sizeof(float) * 2);
  const u8* floatUVs = MemoryFromPartData(dataCur, secSizeCur);

  for (u32 i = 0; i < numFloatUVs; ++i) {
    const auto* norm = reinterpret_cast<const float*>(floatUVs + (i * (sizeof(float) * 2)));
    m_floatUVs.emplace_back(hecl::SBig(norm[0]), hecl::SBig(norm[1]));
  }

  if ((flags & 4) != 0) {
    u32 numShortUVs = hecl::SBig(*reinterpret_cast<const u32*>(secSizeCur)) / (sizeof(s16) * 2);
    const u8* shortUVs = MemoryFromPartData(dataCur, secSizeCur);

    for (u32 i = 0; i < numShortUVs; ++i) {
      const auto* norm = reinterpret_cast<const s16*>(shortUVs + (i * (sizeof(s16) * 2)));
      m_shortUVs.emplace_back(std::array{hecl::SBig(norm[0]), hecl::SBig(norm[1])});
    }
  }

  const u8* surfaceInfo = MemoryFromPartData(dataCur, secSizeCur);
  u32 surfaceCount = hecl::SBig(*reinterpret_cast<const u32*>(surfaceInfo));
  x8_surfaces.reserve(surfaceCount);

  for (u32 i = 0; i < surfaceCount; ++i) {
    if (x8_surfaces.capacity() <= x8_surfaces.size()) {
      x8_surfaces.reserve(x8_surfaces.capacity() * 2);
    }

    x8_surfaces.emplace_back(MemoryFromPartData(dataCur, secSizeCur));
  }

  const float* bounds = reinterpret_cast<float*>(data + 12);
  m_aabox.min = {hecl::SBig(bounds[0]), hecl::SBig(bounds[1]), hecl::SBig(bounds[2])};
  m_aabox.max = {hecl::SBig(bounds[3]), hecl::SBig(bounds[4]), hecl::SBig(bounds[5])};

  /* This constructor has been changed from the original to take into account platform differences */
  x28_modelInst = std::make_unique<CCubeModel>(&x8_surfaces, &x18_matSets[0].x0_textures, x18_matSets[0].x10_data,
                                               &m_positions, &m_floatNormals, &m_shortNormals, &m_colors, &m_floatUVs,
                                               &m_shortUVs, &m_aabox, flags, true, -1);

  sThisFrameList = this;
  if (x34_next != nullptr) {
    x34_next->x30_prev = this;
  }
  x4_dataLen += x8_surfaces.size() * 4;
  sTotalMemory += x4_dataLen;
  // DCFlushRange(x0_data, dataLen);
}

void CModel::UpdateLastFrame() { x38_lastFrame = CGraphics::GetFrameCounter(); }
void CModel::MoveToThisFrameList() {
  UpdateLastFrame();
  if (sThisFrameList == this) {
    return;
  }

  RemoveFromList();
  if (sThisFrameList != nullptr) {
    x34_next = sThisFrameList;
    x34_next->x30_prev = this;
  }

  sThisFrameList = this;
}

void CModel::RemoveFromList() {
  if (x30_prev == nullptr) {
    if (sThisFrameList == this) {
      sThisFrameList = x34_next;
    } else if (sOneFrameList == this) {
      sOneFrameList = x34_next;
    } else if (sTwoFrameList == this) {
      sTwoFrameList = x34_next;
    }
  } else {
    x30_prev->x34_next = x34_next;
  }
  if (x34_next != nullptr) {
    x34_next->x30_prev = x30_prev;
  }
  x30_prev = nullptr;
  x34_next = nullptr;
}

void CModel::FrameDone() {
  ++sFrameCounter;
  if (sIsTextureTimeoutEnabled) {
    auto* iter = sTwoFrameList;
    while (iter != nullptr) {
      auto* next = iter->x34_next;
      iter->VerifyCurrentShader(0);
      for (auto& shader : iter->x18_matSets) {
        shader.UnlockTextures();
      }

      iter->x28_modelInst->UnlockTextures();
      iter->x34_next = nullptr;
      iter->x30_prev = nullptr;
      iter = next;
    }

    sTwoFrameList = sOneFrameList;
    sOneFrameList = sThisFrameList;
    sThisFrameList = nullptr;
  }
}

void CModel::EnableTextureTimeout() { sIsTextureTimeoutEnabled = true; }
void CModel::DisableTextureTimeout() { sIsTextureTimeoutEnabled = false; }
#pragma endregion

#pragma region CCubeModel

void CCubeModel::UnlockTextures() {}

void CCubeModel::MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& texture, bool b1) {}

#pragma endregion

#pragma region CCubeSurface
CCubeSurface::CCubeSurface(u8* ptr) {}
#pragma endregion

CFactoryFnReturn FModelFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  CFactoryFnReturn ret = TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, sp));
  return ret;
}

} // namespace metaforce
