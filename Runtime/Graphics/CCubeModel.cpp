#include "CCubeModel.hpp"

#include "CGraphics.hpp"

namespace metaforce {

#pragma region CModel
u32 CModel::sTotalMemory = 0;
u32 CModel::sFrameCounter = 0;
bool CModel::sIsTextureTimeoutEnabled = true;
CModel* CModel::sThisFrameList = nullptr;
CModel* CModel::sOneFrameList = nullptr;
CModel* CModel::sTwoFrameList = nullptr;

static const u8* MemoryFromPartData(const u8*& dataCur, const u32*& secSizeCur) {
  const u8* ret = nullptr;
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
  const u8* data = reinterpret_cast<const u8*>(x0_data.get());
  u32 flags = *reinterpret_cast<const u32*>(data + 8);
  u32 sectionSizeStart = 0x2c;
  if (hecl::SBig(*reinterpret_cast<const u32*>(data + 4)) == 1) {
    sectionSizeStart = 0x28;
  }
  const u32* secSizeCur = reinterpret_cast<const u32*>(data + sectionSizeStart);
  s32 numMatSets = 1;
  if (hecl::SBig(*reinterpret_cast<const u32*>(data + 4)) > 1) {
    numMatSets = hecl::SBig(*reinterpret_cast<const s32*>(data + 0x28));
  }
  const u8* dataCur = data + ROUND_UP_32(sectionSizeStart + hecl::SBig(*reinterpret_cast<const s32*>(data + 0x24)) * 4);
  x18_matSets.reserve(numMatSets);
  for (s32 i = 0; i < numMatSets; ++i) {
    x18_matSets.emplace_back(static_cast<const u8*>(MemoryFromPartData(dataCur, secSizeCur)));
    auto shader = x18_matSets.back();
    CCubeModel::MakeTexturesFromMats(shader.x10_data, shader.x0_textures, true);
    x4_dataLen += shader.x0_textures.size() * sizeof(TCachedToken<CTexture>);
  }

  const u8* positions = MemoryFromPartData(dataCur, secSizeCur);
  const u8* normals = MemoryFromPartData(dataCur, secSizeCur);
  const u8* vtxColors = MemoryFromPartData(dataCur, secSizeCur);
  const u8* floatUVs = MemoryFromPartData(dataCur, secSizeCur);
  const u8* shortUVs = nullptr;
  if (((flags >> 2) & 1) != 0) {
    shortUVs = MemoryFromPartData(dataCur, secSizeCur);
  }

  const u8* surfaceInfo = MemoryFromPartData(dataCur, secSizeCur);
  u32 surfaceCount = hecl::SBig(*reinterpret_cast<const u32*>(surfaceInfo));
  x8_surfaces.reserve(surfaceCount);

  for (u32 i = 0; i < surfaceCount; ++i) {
    // Implement CCubeSurface loading taking into account endian
  }

  // TODO: need to endian swap the values
  const auto* aabox = reinterpret_cast<const zeus::CAABox*>(data + 12);
  x28_modelInst =
      std::make_unique<CCubeModel>(&x8_surfaces, &x18_matSets[0].x0_textures, x18_matSets[0].x10_data, positions,
                                   normals, vtxColors, floatUVs, shortUVs, aabox, flags, true, -1);

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
  auto* iter = sTwoFrameList;
  if (sIsTextureTimeoutEnabled) {
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

} // namespace metaforce
