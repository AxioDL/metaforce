#include "Runtime/AutoMapper/CMapArea.hpp"

#include <array>
#include <cstring>

#include "Runtime/AutoMapper/CMappableObject.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/CBasics.hpp"
#include "Runtime/Graphics/CGX.hpp"

namespace metaforce {
constexpr std::array<zeus::CVector3f, 3> MinesPostTransforms{{
    {0.f, 0.f, 200.f},
    {0.f, 0.f, 0.f},
    {0.f, 0.f, -200.f},
}};

constexpr std::array<u8, 42> MinesPostTransformIndices{
    0, // 00 Transport to Tallon Overworld South
    0, // 01 Quarry Access
    0, // 02 Main Quarry
    0, // 03 Waste Disposal
    0, // 04 Save Station Mines A
    0, // 05 Security Access A
    0, // 06 Ore Processing
    0, // 07 Mine Security Station
    0, // 08 Research Access
    0, // 09 Storage Depot B
    0, // 10 Elevator Access A
    0, // 11 Security Access B
    0, // 12 Storage Depot A
    0, // 13 Elite Research
    0, // 14 Elevator A
    1, // 15 Elite Control Access
    1, // 16 Elite Control
    1, // 17 Maintenance Tunnel
    1, // 18 Ventilation Shaft
    2, // 19 Phazon Processing Center
    1, // 20 Omega Research
    2, // 21 Transport Access
    2, // 22 Processing Center Access
    1, // 23 Map Station Mines
    1, // 24 Dynamo Access
    2, // 25 Transport to Magmoor Caverns South
    2, // 26 Elite Quarters
    1, // 27 Central Dynamo
    2, // 28 Elite Quarters Access
    1, // 29 Quarantine Access A
    1, // 30 Save Station Mines B
    2, // 31 Metroid Quarantine B
    1, // 32 Metroid Quarantine A
    2, // 33 Quarantine Access B
    2, // 34 Save Station Mines C
    1, // 35 Elevator Access B
    2, // 36 Fungal Hall B
    1, // 37 Elevator B
    2, // 38 Missile Station Mines
    2, // 39 Phazon Mining Tunnel
    2, // 40 Fungal Hall Access
    2, // 41 Fungal Hall A
};

CMapArea::CMapArea(CInputStream& in, u32 size)
: x0_magic(in.ReadLong())
, x4_version(in.ReadLong())
, x8_(in.ReadLong())
, xc_visibilityMode(EVisMode(in.ReadLong()))
, x10_box(in.Get<zeus::CAABox>())
, x28_mappableObjCount(in.ReadLong())
, x2c_vertexCount(in.ReadLong())
, x30_surfaceCount(in.ReadLong())
, x34_size(size - 52) {
  x44_buf.reset(new u8[x34_size]);
  in.ReadBytes(x44_buf.get(), x34_size);
  PostConstruct();
}

void CMapArea::PostConstruct() {
//  OPTICK_EVENT();
  x38_moStart = x44_buf.get();
  x3c_vertexStart = x38_moStart + (x28_mappableObjCount * 0x50);
  x40_surfaceStart = x3c_vertexStart + (x2c_vertexCount * 12);

  m_mappableObjects.reserve(x28_mappableObjCount);
  for (u32 i = 0, j = 0; i < x28_mappableObjCount; ++i, j += 0x50) {
    m_mappableObjects.emplace_back(x38_moStart + j).PostConstruct(x44_buf.get());
  }

  u8* tmp = x3c_vertexStart;
  m_verts.reserve(x2c_vertexCount);
  for (u32 i = 0; i < x2c_vertexCount; ++i) {
    float x;
    std::memcpy(&x, tmp, sizeof(float));
    float y;
    std::memcpy(&y, tmp + 4, sizeof(float));
    float z;
    std::memcpy(&z, tmp + 8, sizeof(float));

    m_verts.emplace_back(CBasics::SwapBytes(x), CBasics::SwapBytes(y), CBasics::SwapBytes(z));
    tmp += 12;
  }

  m_surfaces.reserve(x30_surfaceCount);
  for (u32 i = 0, j = 0; i < x30_surfaceCount; ++i, j += 32) {
    m_surfaces.emplace_back(x40_surfaceStart + j).PostConstruct(x44_buf.get());
  }
}

bool CMapArea::GetIsVisibleToAutoMapper(bool worldVis, bool areaVis) const {
  switch (xc_visibilityMode) {
  case EVisMode::Always:
    return true;
  case EVisMode::MapStationOrVisit:
    return worldVis || areaVis;
  case EVisMode::Visit:
    return areaVis;
  case EVisMode::Never:
    return false;
  default:
    return true;
  }
}

zeus::CTransform CMapArea::GetAreaPostTransform(const IWorld& world, TAreaId aid) const {
  if (world.IGetWorldAssetId() == 0xB1AC4D65) // Phazon Mines
  {
    const zeus::CTransform& areaXf = world.IGetAreaAlways(aid)->IGetTM();
    const zeus::CVector3f& postVec = MinesPostTransforms[MinesPostTransformIndices[aid]];
    return zeus::CTransform::Translate(postVec) * areaXf;
  } else {
    return world.IGetAreaAlways(aid)->IGetTM();
  }
}

const zeus::CVector3f& CMapArea::GetAreaPostTranslate(const IWorld& world, TAreaId aid) {
  if (world.IGetWorldAssetId() == 0xB1AC4D65) // Phazon Mines
    return MinesPostTransforms[MinesPostTransformIndices[aid]];
  else
    return zeus::skZero3f;
}

CMapArea::CMapAreaSurface::CMapAreaSurface(const void* surfBuf) {
  CMemoryInStream r(surfBuf, 32, CMemoryInStream::EOwnerShip::NotOwned);
  x0_normal = r.Get<zeus::CVector3f>();
  xc_centroid = r.Get<zeus::CVector3f>();
  x18_surfOffset = reinterpret_cast<const u32*>(static_cast<uintptr_t>(r.ReadLong()));
  x1c_outlineOffset = reinterpret_cast<const u32*>(static_cast<uintptr_t>(r.ReadLong()));
}

void CMapArea::CMapAreaSurface::PostConstruct(const void* buf) {
  x18_surfOffset =
      reinterpret_cast<const u32*>(static_cast<const u8*>(buf) + reinterpret_cast<uintptr_t>(x18_surfOffset));
  x1c_outlineOffset =
      reinterpret_cast<const u32*>(static_cast<const u8*>(buf) + reinterpret_cast<uintptr_t>(x1c_outlineOffset));
}

void CMapArea::CMapAreaSurface::Draw(TConstVectorRef verts, const CColor& surfColor, const CColor& lineColor,
                                     float lineWidth) const {
  bool hasSurfAlpha = surfColor.a() > 0.0f;
  bool hasLineAlpha = lineColor.a() > 0.0f;
  u32 numSurfaces = CBasics::SwapBytes(*x18_surfOffset);
  u32 numOutlines = CBasics::SwapBytes(*x1c_outlineOffset);
  if (!verts.empty()) {
    CGX::SetArray(GX_VA_POS, verts);
  }
  if (hasSurfAlpha) {
    CGX::SetTevKColor(GX_KCOLOR0, surfColor);
    const u32* surface = &x18_surfOffset[1];
    for (u32 i = 0; i < numSurfaces; ++i) {
      GXPrimitive primType = static_cast<GXPrimitive>(CBasics::SwapBytes(*surface++));
      u32 numVertices = CBasics::SwapBytes(*surface++);
      const u8* data = reinterpret_cast<const u8*>(surface);
      surface += ((numVertices + 3) & ~3) / 4;

      CGX::Begin(primType, GX_VTXFMT0, numVertices);
      for (u32 v = 0; v < numVertices; ++v) {
        GXPosition1x8(data[v]);
      }
      CGX::End();
    }
  }
  if (hasLineAlpha) {
    bool thickLine = lineWidth > 1.f;
    for (u32 j = 0; j < (thickLine ? 1 : 0) + 1; ++j) {
      const u32* outline = &x1c_outlineOffset[1];

      if (thickLine) {
        CGraphics::SetLineWidth(lineWidth - j, ERglTexOffset::One);
      }
      CColor clr = lineColor;
      if (thickLine) {
        clr.a() *= 0.5f;
      }
      CGX::SetTevKColor(GX_KCOLOR0, clr);

      for (u32 i = 0; i < numOutlines; ++i) {
        u32 numVertices = CBasics::SwapBytes(*outline++);
        const u8* data = reinterpret_cast<const u8*>(outline);
        outline += ((numVertices + 3) & ~3) / 4;

        // TODO lines
        // CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, numVertices);
        // for (u32 v = 0; v < numVertices; ++v) {
        //   GXPosition1x8(data[v]);
        // }
        // CGX::End();
      }
    }
  }
}

void CMapArea::CMapAreaSurface::SetupGXMaterial() {
  const GXVtxDescList list[2] = {
      {GX_VA_POS, GX_INDEX8},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(list);
  CGX::SetNumChans(1);
  CGX::SetNumTexGens(0);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_KONST);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
}

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&,
                                 CObjectReference*) {
  u32 size = g_ResFactory->ResourceSize(objTag);
  return TToken<CMapArea>::GetIObjObjectFor(std::make_unique<CMapArea>(in, size));
}

} // namespace metaforce
