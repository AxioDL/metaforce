#include "CMapArea.hpp"
#include "GameGlobalObjects.hpp"
#include "CMappableObject.hpp"
#include "CToken.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"
#include "CResFactory.hpp"

namespace urde {
CMapArea::CMapArea(CInputStream& in, u32 size)
: x0_magic(in.readUint32())
, x4_version(in.readUint32Big())
, x8_(in.readUint32Big())
, xc_visibilityMode(EVisMode(in.readUint32Big()))
, x10_box(zeus::CAABox::ReadBoundingBoxBig(in))
, x28_mappableObjCount(in.readUint32Big())
, x2c_vertexCount(in.readUint32Big())
, x30_surfaceCount(in.readUint32Big())
, x34_size(size - 52) {
  x44_buf.reset(new u8[x34_size]);
  in.readUBytesToBuf(x44_buf.get(), x34_size);
  PostConstruct();
}

void CMapArea::PostConstruct() {
  x38_moStart = x44_buf.get();
  x3c_vertexStart = x38_moStart + (x28_mappableObjCount * 0x50);
  x40_surfaceStart = x3c_vertexStart + (x2c_vertexCount * 12);

  m_mappableObjects.reserve(x28_mappableObjCount);
  for (u32 i = 0, j = 0; i < x28_mappableObjCount; ++i, j += 0x50) {
    m_mappableObjects.emplace_back(x38_moStart + j);
    m_mappableObjects.back().PostConstruct(x44_buf.get());
  }

  u8* tmp = x3c_vertexStart;
  m_verts.reserve(x2c_vertexCount);
  for (u32 i = 0; i < x2c_vertexCount; ++i) {
    float* fl = reinterpret_cast<float*>(tmp);
    m_verts.emplace_back(hecl::SBig(fl[0]), hecl::SBig(fl[1]), hecl::SBig(fl[2]));
    tmp += 12;
  }

  std::vector<u32> index;
  m_surfaces.reserve(x30_surfaceCount);
  for (u32 i = 0, j = 0; i < x30_surfaceCount; ++i, j += 32) {
    m_surfaces.emplace_back(x40_surfaceStart + j);
    m_surfaces.back().PostConstruct(x44_buf.get(), index);
  }

  CGraphics::CommitResources([this, &index](boo::IGraphicsDataFactory::Context& ctx) {
    m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, m_verts.data(), 16, m_verts.size());
    m_ibo = ctx.newStaticBuffer(boo::BufferUse::Index, index.data(), 4, index.size());

    /* Only the map universe specifies Always; it draws a maximum of 1016 instances */
    size_t instCount = (xc_visibilityMode == EVisMode::Always) ? 1024 : 1;

    for (u32 i = 0; i < x30_surfaceCount; ++i) {
      CMapAreaSurface& surf = m_surfaces[i];
      surf.m_instances.reserve(instCount);
      for (u32 inst = 0; inst < instCount; ++inst) {
        surf.m_instances.emplace_back(ctx, m_vbo, m_ibo);
        CMapAreaSurface::Instance& instance = surf.m_instances.back();

        athena::io::MemoryReader r(surf.x1c_outlineOffset, INT_MAX);
        u32 outlineCount = r.readUint32Big();

        std::vector<CLineRenderer>& linePrims = instance.m_linePrims;
        linePrims.reserve(outlineCount * 2);
        for (u32 j = 0; j < 2; ++j) {
          r.seek(4, athena::SeekOrigin::Begin);
          for (u32 i = 0; i < outlineCount; ++i) {
            u32 count = r.readUint32Big();
            r.seek(count);
            r.seekAlign4();
            linePrims.emplace_back(ctx, CLineRenderer::EPrimitiveMode::LineStrip, count, nullptr, false, false, true);
          }
        }
      }
    }

    for (u32 i = 0; i < x28_mappableObjCount; ++i) {
      CMappableObject& mapObj = m_mappableObjects[i];
      if (CMappableObject::IsDoorType(mapObj.GetType()))
        mapObj.CreateDoorSurface(ctx);
    }
    return true;
  } BooTrace);
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

static const zeus::CVector3f MinesPostTransforms[3] = {{0.f, 0.f, 200.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, -200.f}};
static const u8 MinesPostTransformIndices[] = {
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
  athena::io::MemoryReader r(surfBuf, 32);
  x0_normal = r.readVec3fBig();
  xc_centroid = r.readVec3fBig();
  x18_surfOffset = reinterpret_cast<const u8*>(uintptr_t(r.readUint32Big()));
  x1c_outlineOffset = reinterpret_cast<const u8*>(uintptr_t(r.readUint32Big()));
}

void CMapArea::CMapAreaSurface::PostConstruct(const u8* buf, std::vector<u32>& index) {
  x18_surfOffset = buf + reinterpret_cast<uintptr_t>(x18_surfOffset);
  x1c_outlineOffset = buf + reinterpret_cast<uintptr_t>(x1c_outlineOffset);

  m_primStart = index.size();
  bool start = true;
  {
    athena::io::MemoryReader r(x18_surfOffset, INT_MAX);
    u32 primCount = r.readUint32Big();
    for (u32 i = 0; i < primCount; ++i) {
      GX::Primitive prim = GX::Primitive(r.readUint32Big());
      u32 count = r.readUint32Big();
      switch (prim) {
      case GX::Primitive::TRIANGLES: {
        for (u32 v = 0; v < count; v += 3) {
          if (!start) {
            index.push_back(index.back());
            index.push_back(r.readUByte());
            index.push_back(index.back());
          } else {
            index.push_back(r.readUByte());
            start = false;
          }
          index.push_back(r.readUByte());
          index.push_back(r.readUByte());
          index.push_back(index.back());
        }
        break;
      }
      case GX::Primitive::TRIANGLESTRIP: {
        if (!start) {
          index.push_back(index.back());
          index.push_back(r.readUByte());
          index.push_back(index.back());
        } else {
          index.push_back(r.readUByte());
          start = false;
        }
        for (u32 v = 1; v < count; ++v)
          index.push_back(r.readUByte());
        if (count & 1)
          index.push_back(index.back());
        break;
      }
      case GX::Primitive::TRIANGLEFAN: {
        u8 firstVert = r.readUByte();
        if (!start) {
          index.push_back(index.back());
          index.push_back(r.readUByte());
        } else {
          index.push_back(r.readUByte());
          index.push_back(index.back());
          start = false;
        }
        for (u32 v = 1; v < count; ++v) {
          index.push_back(firstVert);
          index.push_back(r.readUByte());
        }
        break;
      }
      default:
        break;
      }
      r.seekAlign4();
    }
  }
  m_primCount = index.size() - m_primStart;
}

void CMapArea::CMapAreaSurface::Draw(const zeus::CVector3f* verts, const zeus::CColor& surfColor,
                                     const zeus::CColor& lineColor, float lineWidth, int instIdx) const {
  if (instIdx >= m_instances.size())
    return;

  Instance& instance = const_cast<Instance&>(m_instances[instIdx]);

  if (surfColor.a())
    instance.m_surfacePrims.draw(surfColor, m_primStart, m_primCount);

  if (lineColor.a()) {
    bool draw2 = lineWidth > 1.f;
    athena::io::MemoryReader r(x1c_outlineOffset, INT_MAX);
    u32 outlineCount = r.readUint32Big();

    std::vector<CLineRenderer>& linePrims = instance.m_linePrims;
    zeus::CColor color = lineColor;
    if (draw2)
      color.a() *= 0.5f;
    float width = lineWidth;

    auto primIt = linePrims.begin();
    for (u32 j = 0; j <= u32(draw2); ++j) {
      r.seek(4, athena::SeekOrigin::Begin);
      for (u32 i = 0; i < outlineCount; ++i) {
        CLineRenderer& prim = *primIt++;
        prim.Reset();
        u32 count = r.readUint32Big();
        for (u32 v = 0; v < count; ++v) {
          u8 idx = r.readUByte();
          prim.AddVertex(verts[idx], color, width);
        }
        r.seekAlign4();
        prim.Render();
      }
      width -= 1.f;
    }
  }
}

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&,
                                 CObjectReference*) {
  u32 size = g_ResFactory->ResourceSize(objTag);
  return TToken<CMapArea>::GetIObjObjectFor(std::make_unique<CMapArea>(in, size));
}

} // namespace urde
