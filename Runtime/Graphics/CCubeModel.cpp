#include "CCubeModel.hpp"

#include "CGraphics.hpp"

namespace metaforce {
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
: x0_data(std::move(in)), x4_dataLen(dataLen), x38_lastFrame(CGraphics::GetFrameCounter() - 2) {
  const u8* data = reinterpret_cast<const u8*>(x0_data.get());
  u32 uVar1 = *reinterpret_cast<const u32*>(data + 8);
  u32 sectionSizeStart = 0x2c;
  if (*reinterpret_cast<const u32*>(data + 4) == 1) {
    sectionSizeStart = 0x28;
  }
  const u32* secSizeCur = reinterpret_cast<const u32*>(data + sectionSizeStart);
  s32 numMatSets = 1;
  if (*reinterpret_cast<const u32*>(data + 4) > 1) {
    numMatSets = *reinterpret_cast<const s32*>(data + 0x28);
  }
  const auto* dataCur = data + (sectionSizeStart + *reinterpret_cast<const s32*>(data + 0x24) * 4 + 0x1f & 0xffffffe0);
  x18_matSets.reserve(numMatSets);
  for (int i = 0; i < numMatSets; i += 1) {
    x18_matSets.emplace_back(
        const_cast<CCubeModel*>(reinterpret_cast<const CCubeModel*>(MemoryFromPartData(dataCur, secSizeCur))));
  }
}
} // namespace metaforce
