#include "CCubeModel.hpp"

#include "CGraphics.hpp"

namespace metaforce {
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
: x0_data(std::move(in)), x4_dataLen(dataLen), x34_next(sThisFrameList), x38_lastFrame(CGraphics::GetFrameCounter() - 2) {
  const u8* data = reinterpret_cast<const u8*>(x0_data.get());
  u32 uVar1 = *reinterpret_cast<const u32*>(data + 8);
  u32 sectionSizeStart = 0x2c;
  if (hecl::SBig(*reinterpret_cast<const u32*>(data + 4)) == 1) {
    sectionSizeStart = 0x28;
  }
  const u32* secSizeCur = reinterpret_cast<const u32*>(data + sectionSizeStart);
  s32 numMatSets = 1;
  if (hecl::SBig(*reinterpret_cast<const u32*>(data + 4)) > 1) {
    numMatSets = hecl::SBig(*reinterpret_cast<const s32*>(data + 0x28));
  }
  const auto* dataCur = data + ROUND_UP_32(sectionSizeStart + hecl::SBig(*reinterpret_cast<const s32*>(data + 0x24)) * 4);
  x18_matSets.reserve(numMatSets);
  for (s32 i = 0; i < numMatSets; ++i) {
    x18_matSets.emplace_back(static_cast<const u8*>(MemoryFromPartData(dataCur, secSizeCur)));
    auto shader = x18_matSets.back();
    CCubeModel::MakeTexturesFromMats(shader.x10_data, shader.x0_textures, true);
  }
}

void CModel::UpdateLastFrame() { x38_lastFrame = CGraphics::GetFrameCounter(); }
void CModel::MoveToThisFrameList() {
  UpdateLastFrame();
  CModel* ptr = sThisFrameList;
  if (sThisFrameList != this) {
    RemoveFromList();
    if (sThisFrameList != nullptr) {
      x34_next = sThisFrameList;
      x34_next->x30_prev = this;
    }
    ptr = this;
  }

  sThisFrameList = ptr;
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
} // namespace metaforce
