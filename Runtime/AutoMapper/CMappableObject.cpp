#include "Runtime/AutoMapper/CMappableObject.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/AutoMapper/CMapWorldInfo.hpp"
#include "Runtime/Camera/CCameraFilter.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CTexture.hpp"

namespace urde {
std::array<zeus::CVector3f, 8> CMappableObject::skDoorVerts{};

constexpr std::array<u32, 24> DoorIndices{
    6, 4, 2, 0, 3, 1, 7, 5, 1, 0, 5, 4, 7, 6, 3, 2, 3, 2, 1, 0, 5, 4, 7, 6,
};

CMappableObject::CMappableObject(const void* buf) {
  athena::io::MemoryReader r(buf, 64);
  x0_type = EMappableObjectType(r.readUint32Big());
  x4_visibilityMode = EVisMode(r.readUint32Big());
  x8_objId = r.readUint32Big();
  xc_ = r.readUint32Big();
  x10_transform.read34RowMajor(r);
}

zeus::CTransform CMappableObject::AdjustTransformForType() const {
  const float doorCenterX = g_tweakAutoMapper->GetDoorCenter().x();
  const float doorCenterZ = g_tweakAutoMapper->GetDoorCenter().z();
  if (x0_type == EMappableObjectType::BigDoor1) {
    zeus::CTransform orientation;
    orientation.origin = {-1.4f * doorCenterX, 0.0f, 0.0f};
    orientation.rotateLocalZ(zeus::degToRad(90.0f));
    return (x10_transform * orientation) * zeus::CTransform::Scale(zeus::CVector3f{1.5f});
  } else if (x0_type == EMappableObjectType::BigDoor2) {
    zeus::CTransform orientation;
    orientation.origin = {0.f, -2.0f * doorCenterZ, -1.4f * doorCenterX};
    orientation.rotateLocalZ(zeus::degToRad(-90.f));
    return (x10_transform * orientation) * zeus::CTransform::Scale(zeus::CVector3f{1.5f});
  } else if (x0_type == EMappableObjectType::IceDoorCeiling || x0_type == EMappableObjectType::WaveDoorCeiling ||
             x0_type == EMappableObjectType::PlasmaDoorCeiling) {
    zeus::CTransform orientation;
    orientation.origin = {-1.65f * doorCenterX, 0.f, -1.5f * doorCenterZ};
    orientation.rotateLocalY(zeus::degToRad(90.f));
    return x10_transform * orientation;
  } else if (x0_type == EMappableObjectType::IceDoorFloor || x0_type == EMappableObjectType::WaveDoorFloor ||
             x0_type == EMappableObjectType::PlasmaDoorFloor) {
    zeus::CTransform orientation;
    orientation.origin = {-1.65f * doorCenterX, 0.f, -1.f * doorCenterZ};
    orientation.rotateLocalY(zeus::degToRad(90.f));
    return x10_transform * orientation;
  } else if ((u32(x0_type) - u32(EMappableObjectType::IceDoorFloor2)) <= u32(EMappableObjectType::ShieldDoor) ||
             x0_type == EMappableObjectType::PlasmaDoorFloor2) {
    zeus::CTransform orientation;
    orientation.origin = {-0.49f * doorCenterX, 0.f, -1.f * doorCenterZ};
    orientation.rotateLocalY(zeus::degToRad(90.f));
    return x10_transform * orientation;
  } else if (x0_type >= EMappableObjectType::BlueDoor && x0_type <= EMappableObjectType::PlasmaDoorFloor2) {
    return x10_transform;
  }
  return zeus::CTransform::Translate(x10_transform.origin);
}

std::pair<zeus::CColor, zeus::CColor> CMappableObject::GetDoorColors(int curAreaId, const CMapWorldInfo& mwInfo,
                                                                     float alpha) const {
  zeus::CColor color = {1.f, 0.f, 1.f, 1.f};
  if (x8_objId.AreaNum() == curAreaId) {
    if (mwInfo.IsDoorVisited(x8_objId) && x0_type == EMappableObjectType::ShieldDoor) {
      color = g_tweakAutoMapper->GetDoorColor(0);
    } else {
      int colorIdx = 0;
      switch (x0_type) {
      case EMappableObjectType::ShieldDoor:
        colorIdx = 1;
        break;
      case EMappableObjectType::IceDoor:
      case EMappableObjectType::IceDoorCeiling:
      case EMappableObjectType::IceDoorFloor:
      case EMappableObjectType::IceDoorFloor2:
        colorIdx = 2;
        break;
      case EMappableObjectType::WaveDoor:
      case EMappableObjectType::WaveDoorCeiling:
      case EMappableObjectType::WaveDoorFloor:
      case EMappableObjectType::WaveDoorFloor2:
        colorIdx = 3;
        break;
      case EMappableObjectType::PlasmaDoor:
      case EMappableObjectType::PlasmaDoorCeiling:
      case EMappableObjectType::PlasmaDoorFloor:
      case EMappableObjectType::PlasmaDoorFloor2:
        colorIdx = 4;
        break;
      default:
        break;
      }
      color = g_tweakAutoMapper->GetDoorColor(colorIdx);
    }
  } else if (mwInfo.IsDoorVisited(x8_objId)) {
    color = g_tweakAutoMapper->GetOpenDoorColor();
  } else {
    color = zeus::skClear;
  }

  color.a() *= alpha;
  return {color, zeus::CColor(std::min(1.4f * color.r(), 1.f), std::min(1.4f * color.g(), 1.f),
                              std::min(1.4f * color.b(), 1.f), std::min(1.4f * color.a(), 1.f))};
}

void CMappableObject::PostConstruct(const void*) { x10_transform = AdjustTransformForType(); }

void CMappableObject::Draw(int curArea, const CMapWorldInfo& mwInfo, float alpha, bool needsVtxLoad) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CMappableObject::Draw", zeus::skCyan);
  if (IsDoorType(x0_type)) {
    std::pair<zeus::CColor, zeus::CColor> colors = GetDoorColors(curArea, mwInfo, alpha);
    for (int s = 0; s < 6; ++s) {
      DoorSurface& ds = *m_doorSurface;
      ds.m_surface.draw(colors.first, s * 4, 4);
      CLineRenderer& line = ds.m_outline;
      const u32* baseIdx = &DoorIndices[s * 4];
      line.Reset();
      line.AddVertex(skDoorVerts[baseIdx[0]], colors.second, 1.f);
      line.AddVertex(skDoorVerts[baseIdx[1]], colors.second, 1.f);
      line.AddVertex(skDoorVerts[baseIdx[3]], colors.second, 1.f);
      line.AddVertex(skDoorVerts[baseIdx[2]], colors.second, 1.f);
      line.Render();
    }
  } else {
    CAssetId iconRes;
    zeus::CColor iconColor = zeus::skWhite;
    switch (x0_type) {
    case EMappableObjectType::DownArrowYellow:
      iconRes = g_tweakPlayerRes->x10_minesBreakFirstTopIcon;
      iconColor = zeus::CColor{1.f, 1.f, 0.588f, 1.f};
      break;
    case EMappableObjectType::UpArrowYellow:
      iconRes = g_tweakPlayerRes->x14_minesBreakFirstBottomIcon;
      iconColor = zeus::CColor{1.f, 1.f, 0.588f, 1.f};
      break;
    case EMappableObjectType::DownArrowGreen:
      iconRes = g_tweakPlayerRes->x10_minesBreakFirstTopIcon;
      iconColor = zeus::CColor{0.392f, 1.f, 0.588f, 1.f};
      break;
    case EMappableObjectType::UpArrowGreen:
      iconRes = g_tweakPlayerRes->x14_minesBreakFirstBottomIcon;
      iconColor = zeus::CColor{0.392f, 1.f, 0.588f, 1.f};
      break;
    case EMappableObjectType::DownArrowRed:
      iconRes = g_tweakPlayerRes->x10_minesBreakFirstTopIcon;
      iconColor = zeus::CColor{1.f, 0.392f, 0.588f, 1.f};
      break;
    case EMappableObjectType::UpArrowRed:
      iconRes = g_tweakPlayerRes->x14_minesBreakFirstBottomIcon;
      iconColor = zeus::CColor{1.f, 0.392f, 0.588f, 1.f};
      break;
    case EMappableObjectType::SaveStation:
      iconRes = g_tweakPlayerRes->x4_saveStationIcon;
      break;
    case EMappableObjectType::MissileStation:
      iconRes = g_tweakPlayerRes->x8_missileStationIcon;
      break;
    default:
      iconRes = g_tweakPlayerRes->xc_elevatorIcon;
      break;
    }

    iconColor.a() *= alpha;

    TLockedToken<CTexture> tex = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), iconRes});
    if (!m_texQuadFilter || m_texQuadFilter->GetTex().GetObj() != tex.GetObj()) {
      m_texQuadFilter.emplace(EFilterType::Add, tex, CTexturedQuadFilter::ZTest::GEqual);
    }

    constexpr std::array<CTexturedQuadFilter::Vert, 4> verts{{
        {{-2.6f, 0.f, 2.6f}, {0.f, 1.f}},
        {{-2.6f, 0.f, -2.6f}, {0.f, 0.f}},
        {{2.6f, 0.f, 2.6f}, {1.f, 1.f}},
        {{2.6f, 0.f, -2.6f}, {1.f, 0.f}},
    }};
    m_texQuadFilter->drawVerts(iconColor, verts);
  }
}

void CMappableObject::DrawDoorSurface(int curArea, const CMapWorldInfo& mwInfo, float alpha, int surfIdx,
                                      bool needsVtxLoad) {
  std::pair<zeus::CColor, zeus::CColor> colors = GetDoorColors(curArea, mwInfo, alpha);
  DoorSurface& ds = *m_doorSurface;
  ds.m_surface.draw(colors.first, surfIdx * 4, 4);
  CLineRenderer& line = ds.m_outline;
  const u32* baseIdx = &DoorIndices[surfIdx * 4];
  line.Reset();
  line.AddVertex(skDoorVerts[baseIdx[0]], colors.second, 1.f);
  line.AddVertex(skDoorVerts[baseIdx[1]], colors.second, 1.f);
  line.AddVertex(skDoorVerts[baseIdx[3]], colors.second, 1.f);
  line.AddVertex(skDoorVerts[baseIdx[2]], colors.second, 1.f);
  line.Render();
}

zeus::CVector3f CMappableObject::BuildSurfaceCenterPoint(int surfIdx) const {
  const zeus::CVector3f& doorCenter = g_tweakAutoMapper->GetDoorCenter();

  switch (surfIdx) {
  case 0:
    return x10_transform * zeus::skZero3f;
  case 1:
    return x10_transform * zeus::CVector3f{0.f, 0.f, 2.f * doorCenter.x()};
  case 2:
    return x10_transform * zeus::CVector3f{0.f, -doorCenter.y(), 0.f};
  case 3:
    return x10_transform * zeus::CVector3f{0.f, doorCenter.y(), 0.f};
  case 4:
    return x10_transform * zeus::CVector3f{-doorCenter.x(), 0.f, 0.f};
  case 5:
    return x10_transform * zeus::CVector3f{doorCenter.x(), 0.f, 0.f};
  default:
    break;
  }

  return {};
}

bool CMappableObject::IsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& mwInfo) const {
  bool areaVis = mwInfo.IsAreaVisible(x8_objId.AreaNum());
  switch (x4_visibilityMode) {
  case EVisMode::Always:
  default:
    return true;
  case EVisMode::MapStationOrVisit:
  case EVisMode::MapStationOrVisit2:
    return worldVis || areaVis;
  case EVisMode::Visit:
    if (IsDoorType(x0_type))
      return mwInfo.IsDoorVisited(x8_objId);
    return areaVis;
  case EVisMode::Never:
    return false;
  }
}

boo::ObjToken<boo::IGraphicsBufferS> CMappableObject::g_doorVbo;
boo::ObjToken<boo::IGraphicsBufferS> CMappableObject::g_doorIbo;

void CMappableObject::ReadAutoMapperTweaks(const ITweakAutoMapper& tweaks) {
  const zeus::CVector3f& center = tweaks.GetDoorCenter();
  const zeus::simd_floats centerF(center.mSimd);

  // Wrap door verts around -Z to build surface
  auto& doorVerts = skDoorVerts;
  doorVerts[0].assign(-centerF[2], -centerF[1], 0.f);
  doorVerts[1].assign(-centerF[2], -centerF[1], 2.f * centerF[0]);
  doorVerts[2].assign(-centerF[2], centerF[1], 0.f);
  doorVerts[3].assign(-centerF[2], centerF[1], 2.f * centerF[0]);
  doorVerts[4].assign(.2f * -centerF[2], -centerF[1], 0.f);
  doorVerts[5].assign(.2f * -centerF[2], -centerF[1], 2.f * centerF[0]);
  doorVerts[6].assign(.2f * -centerF[2], centerF[1], 0.f);
  doorVerts[7].assign(.2f * -centerF[2], centerF[1], 2.f * centerF[0]);

  CGraphics::CommitResources([](boo::IGraphicsDataFactory::Context& ctx) {
    g_doorVbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, skDoorVerts.data(), 16, skDoorVerts.size());
    g_doorIbo = ctx.newStaticBuffer(boo::BufferUse::Index, DoorIndices.data(), 4, DoorIndices.size());
    return true;
  } BooTrace);
}

void CMappableObject::Shutdown() {
  g_doorVbo.reset();
  g_doorIbo.reset();
}
} // namespace urde
