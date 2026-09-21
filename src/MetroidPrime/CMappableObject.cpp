#include "MetroidPrime/CMappableObject.hpp"

#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"

#include "dolphin/gx/GXEnum.h"

#include "rstl/math.hpp"

struct SDrawData {
  float x0_x;
  float x4_y;
  float x8_z;
  uchar xc_idxA;
  uchar xd_idxB;
  uchar xe_idxC;
  uchar xf_idxD;
};

static const SDrawData skDoorSurfaceInfos[6] = {
    // clang-format off
    { 0.f,  0.f, -1.f, 6, 4, 2, 0},
    { 0.f,  0.f,  1.f, 3, 1, 7, 5},
    { 0.f, -1.f,  1.f, 1, 0, 5, 4},
    { 0.f,  1.f,  1.f, 7, 6, 3, 2},
    {-1.f,  0.f,  0.f, 3, 2, 1, 0},
    { 1.f,  0.f,  0.f, 5, 4, 7, 6},
    // clang-format on
};

static CVector3f skDoorVerts[8] = {
    CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(),
    CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(), CVector3f::Zero(),
};

void CMappableObject::ReadAutomapperTweaks(const CTweakAutoMapper& tweaks) {
  const float x = tweaks.xac_doorCenterC;
  const float y = tweaks.xa8_doorCenterB;
  const float z = tweaks.xa4_doorCenterA;
  skDoorVerts[0] = CVector3f(-x, -y, 0.f);
  skDoorVerts[1] = CVector3f(-x, -y, z * 2.f);
  skDoorVerts[2] = CVector3f(-x, y, 0.f);
  skDoorVerts[3] = CVector3f(-x, y, z * 2.f);
  skDoorVerts[4] = CVector3f(-x * .2f, -y, 0.f);
  skDoorVerts[5] = CVector3f(-x * .2f, -y, z * 2.f);
  skDoorVerts[6] = CVector3f(-x * .2f, y, 0.f);
  skDoorVerts[7] = CVector3f(-x * .2f, y, z * 2.f);
}

rstl::pair< CColor, CColor >
CMappableObject::GetDoorColors(int curAreaId, const CMapWorldInfo& mwInfo, float alpha) const {
  CColor firstColor;
  bool areaNumMatches = x8_objId.AreaNum() == curAreaId;
  bool doorVisited = mwInfo.IsDoorVisited(x8_objId);

  if (areaNumMatches) {
    if (doorVisited && x0_type == kMOT_ShieldDoor) {
      firstColor = gpTweakAutoMapper->x100_doorColors[0];
    } else {
      int colorIdx = 0;
      switch (x0_type) {
      case kMOT_ShieldDoor:
        colorIdx = 1;
        break;
      case kMOT_IceDoor:
      case kMOT_IceDoorCeiling:
      case kMOT_IceDoorFloor:
      case kMOT_IceDoorFloor2:
        colorIdx = 2;
        break;
      case kMOT_WaveDoor:
      case kMOT_WaveDoorCeiling:
      case kMOT_WaveDoorFloor:
      case kMOT_WaveDoorFloor2:
        colorIdx = 3;
        break;
      case kMOT_PlasmaDoor:
      case kMOT_PlasmaDoorCeiling:
      case kMOT_PlasmaDoorFloor:
      case kMOT_PlasmaDoorFloor2:
        colorIdx = 4;
        break;
      default:
        break;
      }
      firstColor = gpTweakAutoMapper->x100_doorColors[colorIdx];
    }
  } else if (doorVisited) {
    firstColor = gpTweakAutoMapper->x11c_openDoorColor;
  } else {
    firstColor = CColor(0);
  }

  firstColor = firstColor.WithAlphaModulatedBy(alpha);
  const CColor secondColor(rstl::min_val(1.0f, firstColor.GetRed() * 1.4f),
                           rstl::min_val(1.0f, firstColor.GetGreen() * 1.4f),
                           rstl::min_val(1.0f, firstColor.GetBlue() * 1.4f),
                           rstl::min_val(1.0f, firstColor.GetAlpha() * 1.4f));
  return rstl::pair< CColor, CColor >(firstColor, secondColor);
}

#if !defined(TARGET_PC)
void CMappableObject::PostConstruct(const void*) {
  for (int i = 0; i < offsetof(CMappableObject, x40_pad) / sizeof(int); ++i) {
    reinterpret_cast< int* >(this)[i] = CBasics::SwapBytes(reinterpret_cast< int* >(this)[i]);
  }
  x10_transform = AdjustTransformForType();
}
#endif

static inline void draw_door_surface(const CColor& firstColor, const CColor& secondColor,
                                     int surfaceIdx, bool needsVtxLoad) {
  const SDrawData& drawData = skDoorSurfaceInfos[surfaceIdx];
  if (needsVtxLoad) {
#if defined(TARGET_PC)
    CGX::SetArray(GX_VA_POS, skDoorVerts, sizeof(skDoorVerts[0]), sizeof(skDoorVerts),
                  TARGET_LITTLE_ENDIAN);
#else
    CGX::SetArray(GX_VA_POS, skDoorVerts, sizeof(skDoorVerts[0]));
#endif
  }

  CGX::SetTevKColor(GX_KCOLOR0, firstColor.GetGXColor());
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition1x8(drawData.xc_idxA);
  GXPosition1x8(drawData.xd_idxB);
  GXPosition1x8(drawData.xe_idxC);
  GXPosition1x8(drawData.xf_idxD);
  CGX::End();

  CGX::SetTevKColor(GX_KCOLOR0, secondColor.GetGXColor());
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 5);
  GXPosition1x8(drawData.xc_idxA);
  GXPosition1x8(drawData.xd_idxB);
  GXPosition1x8(drawData.xf_idxD);
  GXPosition1x8(drawData.xe_idxC);
  GXPosition1x8(drawData.xc_idxA);
  CGX::End();
}

void CMappableObject::Draw(int curArea, const CMapWorldInfo& mwInfo, float alpha,
                           bool needsVtxLoad) const {
  if (IsDoorType(x0_type) == true) {
    rstl::pair< CColor, CColor > colors = GetDoorColors(curArea, mwInfo, alpha);
    for (int i = 0; i < 6; ++i) {
      draw_door_surface(colors.first, colors.second, i, needsVtxLoad);
    }
    return;
  }

  CAssetId iconRes = kInvalidAssetId;
  CColor iconColor = CColor(0xffffffff);
  switch (x0_type) {
  case kMOT_DownArrowYellow:
    iconColor = CColor((uchar)0xff, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->x10_minesBreakFirstTopIcon;
    break;
  case kMOT_UpArrowYellow:
    iconColor = CColor((uchar)0xff, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->x14_minesBreakFirstBottomIcon;
    break;
  case kMOT_DownArrowGreen:
    iconColor = CColor((uchar)0x64, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->x10_minesBreakFirstTopIcon;
    break;
  case kMOT_UpArrowGreen:
    iconColor = CColor((uchar)0x64, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->x14_minesBreakFirstBottomIcon;
    break;
  case kMOT_DownArrowRed:
    iconColor = CColor((uchar)0xff, 0x64, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->x10_minesBreakFirstTopIcon;
    break;
  case kMOT_UpArrowRed:
    iconColor = CColor((uchar)0xff, 0x64, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->x14_minesBreakFirstBottomIcon;
    break;
  case kMOT_SaveStation:
    iconRes = gpTweakPlayerRes->x4_saveStationIcon;
    break;
  case kMOT_MissileStation:
    iconRes = gpTweakPlayerRes->x8_missileStationIcon;
    break;
  default:
    iconRes = gpTweakPlayerRes->xc_elevatorIcon;
    break;
  }

  TLockedToken< CTexture > tex = gpSimplePool->GetObj(SObjectTag('TXTR', iconRes));
  tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::StreamBegin(kP_TriangleStrip);
  CGraphics::StreamColor(iconColor.WithAlphaOf(alpha));
  CGraphics::StreamTexcoord(0.0f, 1.0f);
  CGraphics::StreamVertex(-2.6f, 0.0f, 2.6f);
  CGraphics::StreamTexcoord(0.0f, 0.0f);
  CGraphics::StreamVertex(-2.6f, 0.0f, -2.6f);
  CGraphics::StreamTexcoord(1.0f, 1.0f);
  CGraphics::StreamVertex(2.6f, 0.0f, 2.6f);
  CGraphics::StreamTexcoord(1.0f, 0.0f);
  CGraphics::StreamVertex(2.6f, 0.0f, -2.6f);
  CGraphics::StreamEnd();
}

void CMappableObject::DrawDoorSurface(int curAreaId, const CMapWorldInfo& mwInfo, float alpha,
                                      int surfaceIdx, bool needsVtxLoad) const {
  rstl::pair< CColor, CColor > colors = GetDoorColors(curAreaId, mwInfo, alpha);
  draw_door_surface(colors.first, colors.second, surfaceIdx, needsVtxLoad);
}

CVector3f CMappableObject::BuildSurfaceCenterPoint(int surfaceIdx) const {
  const float x = gpTweakAutoMapper->xac_doorCenterC;
  const float y = gpTweakAutoMapper->xa8_doorCenterB;
  const float z = gpTweakAutoMapper->xa4_doorCenterA;
  switch (surfaceIdx) {
  case 0:
    return x10_transform * CVector3f::Zero();
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
    return CVector3f::Zero();
  }
}

bool CMappableObject::GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& mwInfo) const {
  bool areaVis = mwInfo.IsAreaVisible(x8_objId.AreaNum());
  switch (x4_visibilityMode) {
  case kVM_Always:
    return true;
  case kVM_MapStationOrVisit:
  case kVM_MapStationOrVisit2:
    return worldVis || areaVis;
  case kVM_Visit:
    if (IsDoorType(x0_type)) {
      return mwInfo.IsDoorVisited(x8_objId);
    }
    return areaVis;
  case kVM_Never:
    return false;
  default:
    return true;
  }
}

CTransform4f CMappableObject::AdjustTransformForType() const {
  const float doorCenterX = gpTweakAutoMapper->xa4_doorCenterA;
  const float doorCenterZ = gpTweakAutoMapper->xac_doorCenterC;
  if (x0_type == kMOT_BigDoor1) {
    return (GetTransform() * CTransform4f(CMatrix3f::RotateZ(CRelAngle::FromDegrees(90.f)),
                                          CVector3f(0.f, 0.f, doorCenterX * -1.4f))) *
           CTransform4f::Scale(1.5f);
  } else if (x0_type == kMOT_BigDoor2) {
    return (GetTransform() *
            CTransform4f(CMatrix3f::RotateZ(CRelAngle::FromDegrees(-90.f)),
                         CVector3f(0.f, doorCenterZ * -2.f, doorCenterX * -1.4f))) *
           CTransform4f::Scale(1.5f);
  } else if (x0_type == kMOT_IceDoorCeiling || x0_type == kMOT_WaveDoorCeiling ||
             x0_type == kMOT_PlasmaDoorCeiling) {
    return GetTransform() * CTransform4f(CMatrix3f::RotateY(CRelAngle::FromDegrees(90.f)),
                                         CVector3f(doorCenterX * -1.65f, 0.f, doorCenterZ * -1.5f));
  } else if (x0_type == kMOT_IceDoorFloor || x0_type == kMOT_WaveDoorFloor ||
             x0_type == kMOT_PlasmaDoorFloor) {
    return GetTransform() * CTransform4f(CMatrix3f::RotateY(CRelAngle::FromDegrees(90.f)),
                                         CVector3f(doorCenterX * -1.65f, 0.f, doorCenterZ * -1.f));
  } else if (x0_type == kMOT_IceDoorFloor2 || x0_type == kMOT_WaveDoorFloor2 ||
             x0_type == kMOT_PlasmaDoorFloor2) {
    return GetTransform() * CTransform4f(CMatrix3f::RotateY(CRelAngle::FromDegrees(90.f)),
                                         CVector3f(doorCenterX * -0.49f, 0.f, doorCenterZ * -1.f));
  } else if (IsDoorType(x0_type)) {
    return GetTransform();
  }
  return CTransform4f::Translate(GetTransform().GetTranslation());
}
