#include "Runtime/AutoMapper/CMappableObject.hpp"

#include "Runtime/AutoMapper/CMapWorldInfo.hpp"
#include "Runtime/AutoMapper/CMapArea.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/Graphics/CTexture.hpp"

namespace metaforce {
using zeus::CColor;
using zeus::CVector3f;
using uchar = unsigned char;

struct SDrawData {
  float x0_x;
  float x4_y;
  float x8_z;
  uchar xc_idxA;
  uchar xd_idxB;
  uchar xe_idxC;
  uchar xf_idxD;
};

static const SDrawData sDrawData[6] = {
    // clang-format off
  { 0.f,  0.f, -1.f, 6, 4, 2, 0},
  { 0.f,  0.f,  1.f, 3, 1, 7, 5},
  { 0.f, -1.f,  1.f, 1, 0, 5, 4},
  { 0.f,  1.f,  1.f, 7, 6, 3, 2},
  {-1.f,  0.f,  0.f, 3, 2, 1, 0},
  { 1.f,  0.f,  0.f, 5, 4, 7, 6},
    // clang-format on
};

static std::array<aurora::Vec3<float>, 8> skDoorVerts{};

CMappableObject::CMappableObject(const void* buf) {
  CMemoryInStream r(buf, 64);
  x0_type = EMappableObjectType(r.ReadLong());
  x4_visibilityMode = EVisMode(r.ReadLong());
  x8_objId = r.ReadLong();
  xc_ = r.ReadLong();
  x10_transform = r.Get<zeus::CTransform>();
}

zeus::CTransform CMappableObject::AdjustTransformForType() const {
  const float doorCenterX = g_tweakAutoMapper->xa4_doorCenterA;
  const float doorCenterZ = g_tweakAutoMapper->xac_doorCenterC;
  if (x0_type == EMappableObjectType::BigDoor1) {
    zeus::CTransform orientation;
    orientation.origin = {0.0f, 0.0f, -1.4f * doorCenterX};
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
  } else if (IsDoorType(x0_type)) {
    return x10_transform;
  }
  return zeus::CTransform::Translate(x10_transform.origin);
}

std::pair<CColor, CColor> CMappableObject::GetDoorColors(int curAreaId, const CMapWorldInfo& mwInfo,
                                                         float alpha) const {
  CColor color;
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
  return {color, CColor(std::min(1.4f * color.r(), 1.f), std::min(1.4f * color.g(), 1.f),
                        std::min(1.4f * color.b(), 1.f), std::min(1.4f * color.a(), 1.f))};
}

void CMappableObject::PostConstruct(const void*) { x10_transform = AdjustTransformForType(); }

void CMappableObject::Draw(int curArea, const CMapWorldInfo& mwInfo, float alpha, bool needsVtxLoad) {
  SCOPED_GRAPHICS_DEBUG_GROUP("CMappableObject::Draw", zeus::skCyan);
  if (IsDoorType(x0_type) == true) {
    std::pair<CColor, CColor> colors = GetDoorColors(curArea, mwInfo, alpha);
    for (int i = 0; i < 6; ++i) {
      if (needsVtxLoad) {
        CGX::SetArray(GX_VA_POS, skDoorVerts);
      }
      CGX::SetTevKColor(GX_KCOLOR0, colors.first);
      CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
      GXPosition1x8(sDrawData[i].xc_idxA);
      GXPosition1x8(sDrawData[i].xd_idxB);
      GXPosition1x8(sDrawData[i].xe_idxC);
      GXPosition1x8(sDrawData[i].xf_idxD);
      CGX::End();

      CGX::SetTevKColor(GX_KCOLOR0, colors.second);
      CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 5);
      GXPosition1x8(sDrawData[i].xc_idxA);
      GXPosition1x8(sDrawData[i].xd_idxB);
      GXPosition1x8(sDrawData[i].xf_idxD);
      GXPosition1x8(sDrawData[i].xe_idxC);
      GXPosition1x8(sDrawData[i].xc_idxA);
      CGX::End();
    }
    return;
  }

  CAssetId iconRes;
  CColor iconColor = CColor(0xffffffffu);
  switch (x0_type) {
  case EMappableObjectType::DownArrowYellow:
    iconColor = CColor(0xffff96ffu);
    iconRes = g_tweakPlayerRes->x10_minesBreakFirstTopIcon;
    break;
  case EMappableObjectType::UpArrowYellow:
    iconColor = CColor(0xffff96ffu);
    iconRes = g_tweakPlayerRes->x14_minesBreakFirstBottomIcon;
    break;
  case EMappableObjectType::DownArrowGreen:
    iconColor = CColor(0x64ff96ffu);
    iconRes = g_tweakPlayerRes->x10_minesBreakFirstTopIcon;
    break;
  case EMappableObjectType::UpArrowGreen:
    iconColor = CColor(0x64ff96ffu);
    iconRes = g_tweakPlayerRes->x14_minesBreakFirstBottomIcon;
    break;
  case EMappableObjectType::DownArrowRed:
    iconColor = CColor(0xff6496ffu);
    iconRes = g_tweakPlayerRes->x10_minesBreakFirstTopIcon;
    break;
  case EMappableObjectType::UpArrowRed:
    iconColor = CColor(0xff6496ffu);
    iconRes = g_tweakPlayerRes->x14_minesBreakFirstBottomIcon;
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

  TLockedToken<CTexture> tex = g_SimplePool->GetObj(SObjectTag('TXTR', iconRes));
  tex->Load(GX_TEXMAP0, EClampMode::Repeat);
  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
  CGraphics::StreamBegin(ERglPrimitive::TriangleStrip);
  iconColor.a() = alpha;
  CGraphics::StreamColor(iconColor);
  CGraphics::StreamTexcoord(0.0f, 1.0f);
  CGraphics::StreamVertex(-2.6f, 0.0f, 2.6f);
  CGraphics::StreamTexcoord(0.0f, 0.0f);
  CGraphics::StreamVertex(-2.6f, 0.0f, -2.6f);
  CGraphics::StreamTexcoord(1.0f, 1.0f);
  CGraphics::StreamVertex(2.6f, 0.0f, 2.6f);
  CGraphics::StreamTexcoord(1.0f, 0.0f);
  CGraphics::StreamVertex(2.6f, 0.0f, -2.6f);
  CGraphics::StreamEnd();

  // Metaforce addition: restore GX state
  CMapArea::CMapAreaSurface::SetupGXMaterial();
}

void CMappableObject::DrawDoorSurface(int curArea, const CMapWorldInfo& mwInfo, float alpha, int surfIdx,
                                      bool needsVtxLoad) {
  std::pair<CColor, CColor> colors = GetDoorColors(curArea, mwInfo, alpha);
  const SDrawData& drawData = sDrawData[surfIdx];
  if (needsVtxLoad) {
    CGX::SetArray(GX_VA_POS, skDoorVerts);
  }

  CGX::SetTevKColor(GX_KCOLOR0, colors.first);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition1x8(drawData.xc_idxA);
  GXPosition1x8(drawData.xd_idxB);
  GXPosition1x8(drawData.xe_idxC);
  GXPosition1x8(drawData.xf_idxD);
  CGX::End();

  CGX::SetTevKColor(GX_KCOLOR0, colors.second);
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 5);
  GXPosition1x8(drawData.xc_idxA);
  GXPosition1x8(drawData.xd_idxB);
  GXPosition1x8(drawData.xf_idxD);
  GXPosition1x8(drawData.xe_idxC);
  GXPosition1x8(drawData.xc_idxA);
  CGX::End();
}

CVector3f CMappableObject::BuildSurfaceCenterPoint(int surfaceIdx) const {
  const float x = g_tweakAutoMapper->xac_doorCenterC;
  const float y = g_tweakAutoMapper->xa8_doorCenterB;
  const float z = g_tweakAutoMapper->xa4_doorCenterA;
  switch (surfaceIdx) {
  case 0:
    return x10_transform * CVector3f{};
  case 1:
    return x10_transform * CVector3f(0.f, 0.f, 2.f * z);
  case 2:
    return x10_transform * CVector3f(0.f, -y, 0.f);
  case 3:
    return x10_transform * CVector3f(0.f, y, 0.f);
  case 4:
    return x10_transform * CVector3f(-x, 0.f, 0.f);
  case 5:
    return x10_transform * CVector3f(x, 0.f, 0.f);
  default:
    return CVector3f{};
  }
}

bool CMappableObject::GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& mwInfo) const {
  bool areaVis = mwInfo.IsAreaVisible(x8_objId.AreaNum());
  switch (x4_visibilityMode) {
  case EVisMode::Always:
  default:
    return true;
  case EVisMode::MapStationOrVisit:
  case EVisMode::MapStationOrVisit2:
    return worldVis || areaVis;
  case EVisMode::Visit:
    if (IsDoorType(x0_type)) {
      return mwInfo.IsDoorVisited(x8_objId);
    }
    return areaVis;
  case EVisMode::Never:
    return false;
  }
}

void CMappableObject::ReadAutoMapperTweaks(const ITweakAutoMapper& tweaks) {
  const float x = tweaks.xac_doorCenterC;
  const float y = tweaks.xa8_doorCenterB;
  const float z = tweaks.xa4_doorCenterA;
  skDoorVerts[0] = aurora::Vec3(-x, -y, 0.f);
  skDoorVerts[1] = aurora::Vec3(-x, -y, z * 2.f);
  skDoorVerts[2] = aurora::Vec3(-x, y, 0.f);
  skDoorVerts[3] = aurora::Vec3(-x, y, z * 2.f);
  skDoorVerts[4] = aurora::Vec3(-x * .2f, -y, 0.f);
  skDoorVerts[5] = aurora::Vec3(-x * .2f, -y, z * 2.f);
  skDoorVerts[6] = aurora::Vec3(-x * .2f, y, 0.f);
  skDoorVerts[7] = aurora::Vec3(-x * .2f, y, z * 2.f);
}
} // namespace metaforce
