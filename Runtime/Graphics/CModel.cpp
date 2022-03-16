#include "CModel.hpp"

#include "CBasics.hpp"
#include "Graphics/CCubeMaterial.hpp"
#include "Graphics/CCubeModel.hpp"
#include "Graphics/CCubeSurface.hpp"
#include "Streams/IOStreams.hpp"

namespace metaforce {
void CModel::SShader::UnlockTextures() {
  for (auto& token : x0_textures) {
    token.Unlock();
  }
}

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
  dataCur += CBasics::SwapBytes(*secSizeCur);
  ++secSizeCur;
  return ret;
}

// For ease of reading byte swapped data
static CMemoryInStream StreamFromPartData(u8*& dataCur, const u32*& secSizeCur) {
  const auto secSize = CBasics::SwapBytes(*secSizeCur);
  return {MemoryFromPartData(dataCur, secSizeCur), secSize};
}

CModel::CModel(std::unique_ptr<u8[]> in, u32 dataLen, IObjectStore* store)
: x0_data(std::move(in))
, x4_dataLen(dataLen)
, x34_next(sThisFrameList)
, x38_lastFrame(CGraphics::GetFrameCounter() - 2) {
  u8* data = x0_data.get();
  u32 flags = CBasics::SwapBytes(*reinterpret_cast<u32*>(data + 8));
  u32 sectionSizeStart = 0x2c;
  if (CBasics::SwapBytes(*reinterpret_cast<u32*>(data + 4)) == 1) {
    sectionSizeStart = 0x28;
  }
  const u32* secSizeCur = reinterpret_cast<u32*>(data + sectionSizeStart);
  s32 numMatSets = 1;
  if (CBasics::SwapBytes(*reinterpret_cast<u32*>(data + 4)) > 1) {
    numMatSets = CBasics::SwapBytes(*reinterpret_cast<s32*>(data + 0x28));
  }
  u8* dataCur = data + ROUND_UP_32(sectionSizeStart + CBasics::SwapBytes(*reinterpret_cast<s32*>(data + 0x24)) * 4);
  x18_matSets.reserve(numMatSets);
  for (s32 i = 0; i < numMatSets; ++i) {
    x18_matSets.emplace_back(MemoryFromPartData(dataCur, secSizeCur));
    auto& shader = x18_matSets.back();
    CCubeModel::MakeTexturesFromMats(shader.x10_data, shader.x0_textures, store, true);
    x4_dataLen += shader.x0_textures.size() * sizeof(TCachedToken<CTexture>);
  }

  /* Metaforce note: Due to padding in zeus types we need to convert these and store locally */
  u32 numVertices = CBasics::SwapBytes(*secSizeCur) / 12;
  auto positions = StreamFromPartData(dataCur, secSizeCur);
  for (u32 i = 0; i < numVertices; ++i) {
    m_positions.emplace_back(positions.Get<zeus::CVector3f>());
  }

  u32 numNormals = CBasics::SwapBytes(*secSizeCur);
  numNormals /= (flags & 2) == 0 ? 12 : 6;
  auto normals = StreamFromPartData(dataCur, secSizeCur);
  for (u32 i = 0; i < numNormals; ++i) {
    if ((flags & 2) == 0) {
      m_normals.emplace_back(normals.Get<zeus::CVector3f>());
    } else {
      const auto x = static_cast<float>(normals.ReadShort()) / 16384.f;
      const auto y = static_cast<float>(normals.ReadShort()) / 16384.f;
      const auto z = static_cast<float>(normals.ReadShort()) / 16384.f;
      m_normals.emplace_back(x, y, z);
    }
  }

  u32 numColors = CBasics::SwapBytes(*secSizeCur) / 4;
  auto vtxColors = StreamFromPartData(dataCur, secSizeCur);
  for (u32 i = 0; i < numColors; ++i) {
    m_colors.emplace_back(zeus::CColor(vtxColors.ReadUint32()));
  }

  u32 numFloatUVs = CBasics::SwapBytes(*secSizeCur) / 8;
  auto floatUVs = StreamFromPartData(dataCur, secSizeCur);
  for (u32 i = 0; i < numFloatUVs; ++i) {
    m_floatUVs.emplace_back(floatUVs.Get<zeus::CVector2f>());
  }

  if ((flags & 4) != 0) {
    u32 numShortUVs = CBasics::SwapBytes(*secSizeCur) / 4;
    auto shortUVs = StreamFromPartData(dataCur, secSizeCur);
    for (u32 i = 0; i < numShortUVs; ++i) {
      const auto u = static_cast<float>(shortUVs.ReadShort()) / 32768.f;
      const auto v = static_cast<float>(shortUVs.ReadShort()) / 32768.f;
      m_shortUVs.emplace_back(u, v);
    }
  }

  auto surfaceInfo = StreamFromPartData(dataCur, secSizeCur);
  auto surfaceCount = surfaceInfo.ReadUint32();
  x8_surfaces.reserve(surfaceCount);
  for (u32 i = 0; i < surfaceCount; ++i) {
    if (x8_surfaces.capacity() <= x8_surfaces.size()) {
      x8_surfaces.reserve(x8_surfaces.capacity() * 2);
    }
    const auto secSize = CBasics::SwapBytes(*secSizeCur);
    x8_surfaces.emplace_back(MemoryFromPartData(dataCur, secSizeCur), secSize);
  }

  const float* bounds = reinterpret_cast<float*>(data + 12);
  const zeus::CAABox aabb{
      {CBasics::SwapBytes(bounds[0]), CBasics::SwapBytes(bounds[1]), CBasics::SwapBytes(bounds[2])},
      {CBasics::SwapBytes(bounds[3]), CBasics::SwapBytes(bounds[4]), CBasics::SwapBytes(bounds[5])},
  };

  /* This constructor has been changed from the original to take into account platform differences */
  x28_modelInst =
      std::make_unique<CCubeModel>(&x8_surfaces, &x18_matSets[0].x0_textures, x18_matSets[0].x10_data, &m_positions,
                                   &m_colors, &m_normals, &m_floatUVs, &m_shortUVs, aabb, flags, true, -1);

  sThisFrameList = this;
  if (x34_next != nullptr) {
    x34_next->x30_prev = this;
  }
  x4_dataLen += x8_surfaces.size() * 4;
  sTotalMemory += x4_dataLen;
  // DCFlushRange(x0_data, dataLen);
}

CModel::~CModel() {
  RemoveFromList();
  sTotalMemory -= x4_dataLen;
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

TVectorRef CModel::GetPositions() { return x28_modelInst->GetPositions(); }

TConstVectorRef CModel::GetPositions() const { return x28_modelInst->GetPositions(); }

TVectorRef CModel::GetNormals() { return x28_modelInst->GetNormals(); }

TConstVectorRef CModel::GetNormals() const { return x28_modelInst->GetNormals(); }

void CModel::VerifyCurrentShader(u32 matIdx) {
  if (matIdx > x18_matSets.size()) {
    matIdx = 0;
  }
  if (matIdx == x2c_currentMatIdx) {
    if (x2e_lastFrame != 0 && x2e_lastFrame < sFrameCounter) {
      for (size_t idx = 0; auto& mat : x18_matSets) {
        if (idx != matIdx) {
          mat.UnlockTextures();
        }
        idx++;
      }
    }
  } else {
    x2c_currentMatIdx = matIdx;
    auto& mat = x18_matSets[matIdx];
    x28_modelInst->RemapMaterialData(mat.x10_data, mat.x0_textures);
    if (x18_matSets.size() > 1) {
      x2e_lastFrame = sFrameCounter + 2;
    }
  }
}

bool CModel::IsLoaded(u32 matIdx) {
  VerifyCurrentShader(matIdx);
  const auto& textures = *x28_modelInst->x1c_textures;
  if (textures.empty()) {
    return true;
  }
  for (const auto& token : textures) {
    if (token.IsNull() && !token.IsLoaded()) {
      return false;
    }
  }
  return true;
}

void CModel::Touch(u32 matIdx) {
  MoveToThisFrameList();
  VerifyCurrentShader(matIdx);
  if (x28_modelInst->TryLockTextures()) {
    for (auto& texture : *x28_modelInst->x1c_textures) {
      if (!texture.IsNull()) {
        // texture->LoadToMRAM();
      }
    }
  }
}

void CModel::Draw(CModelFlags flags) {
  if (flags.x2_flags & CModelFlagBits::DrawNormal) {
    x28_modelInst->DrawNormal(nullptr, nullptr, ESurfaceSelection::All);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->Draw(flags);
}

void CModel::Draw(TConstVectorRef positions, TConstVectorRef normals, const CModelFlags& flags) {
  if (flags.x2_flags & CModelFlagBits::DrawNormal) {
    x28_modelInst->DrawNormal(positions, normals, ESurfaceSelection::All);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->Draw(positions, normals, flags);
}

void CModel::DrawSortedParts(CModelFlags flags) {
  if (flags.x2_flags & CModelFlagBits::DrawNormal) {
    x28_modelInst->DrawNormal(nullptr, nullptr, ESurfaceSelection::Sorted);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->DrawAlpha(flags);
}

void CModel::DrawUnsortedParts(CModelFlags flags) {
  if (flags.x2_flags & CModelFlagBits::DrawNormal) {
    x28_modelInst->DrawNormal(nullptr, nullptr, ESurfaceSelection::Unsorted);
  }
  CCubeMaterial::ResetCachedMaterials();
  MoveToThisFrameList();
  VerifyCurrentShader(flags.x1_matSetIdx);
  x28_modelInst->DrawNormal(flags);
}

CFactoryFnReturn FModelFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                               const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  IObjectStore* store = vparms.GetOwnedObj<IObjectStore*>();
  CFactoryFnReturn ret = TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, store));
  return ret;
}
} // namespace metaforce
