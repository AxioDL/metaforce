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
  float mX;
  float mY;
  float mZ;
  uchar mIdxA;
  uchar mIdxB;
  uchar mIdxC;
  uchar mIdxD;
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
  const float x = tweaks.mDoorCenterC;
  const float y = tweaks.mDoorCenterB;
  const float z = tweaks.mDoorCenterA;
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
  bool areaNumMatches = mObjId.AreaNum() == curAreaId;
  bool doorVisited = mwInfo.IsDoorVisited(mObjId);

  if (areaNumMatches) {
    if (doorVisited && mType == kMOT_ShieldDoor) {
      firstColor = gpTweakAutoMapper->mDoorColors[0];
    } else {
      int colorIdx = 0;
      switch (mType) {
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
      firstColor = gpTweakAutoMapper->mDoorColors[colorIdx];
    }
  } else if (doorVisited) {
    firstColor = gpTweakAutoMapper->mOpenDoorColor;
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
  for (int i = 0; i < offsetof(CMappableObject, mPad) / sizeof(int); ++i) {
    reinterpret_cast< int* >(this)[i] = CBasics::SwapBytes(reinterpret_cast< int* >(this)[i]);
  }
  mTransform = AdjustTransformForType();
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
  GXPosition1x8(drawData.mIdxA);
  GXPosition1x8(drawData.mIdxB);
  GXPosition1x8(drawData.mIdxC);
  GXPosition1x8(drawData.mIdxD);
  CGX::End();

  CGX::SetTevKColor(GX_KCOLOR0, secondColor.GetGXColor());
  CGX::Begin(GX_LINESTRIP, GX_VTXFMT0, 5);
  GXPosition1x8(drawData.mIdxA);
  GXPosition1x8(drawData.mIdxB);
  GXPosition1x8(drawData.mIdxD);
  GXPosition1x8(drawData.mIdxC);
  GXPosition1x8(drawData.mIdxA);
  CGX::End();
}

void CMappableObject::Draw(int curArea, const CMapWorldInfo& mwInfo, float alpha,
                           bool needsVtxLoad) const {
  if (IsDoorType(mType) == true) {
    rstl::pair< CColor, CColor > colors = GetDoorColors(curArea, mwInfo, alpha);
    for (int i = 0; i < 6; ++i) {
      draw_door_surface(colors.first, colors.second, i, needsVtxLoad);
    }
    return;
  }

  CAssetId iconRes = kInvalidAssetId;
  CColor iconColor = CColor(0xffffffff);
  switch (mType) {
  case kMOT_DownArrowYellow:
    iconColor = CColor((uchar)0xff, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesBreakFirstTopIcon();
    break;
  case kMOT_UpArrowYellow:
    iconColor = CColor((uchar)0xff, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesBreakFirstBottomIcon();
    break;
  case kMOT_DownArrowGreen:
    iconColor = CColor((uchar)0x64, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesBreakFirstTopIcon();
    break;
  case kMOT_UpArrowGreen:
    iconColor = CColor((uchar)0x64, 0xff, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesBreakFirstBottomIcon();
    break;
  case kMOT_DownArrowRed:
    iconColor = CColor((uchar)0xff, 0x64, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesBreakFirstTopIcon();
    break;
  case kMOT_UpArrowRed:
    iconColor = CColor((uchar)0xff, 0x64, 0x96, 0xff);
    iconRes = gpTweakPlayerRes->GetMinesBreakFirstBottomIcon();
    break;
  case kMOT_SaveStation:
    iconRes = gpTweakPlayerRes->GetSaveStationIcon();
    break;
  case kMOT_MissileStation:
    iconRes = gpTweakPlayerRes->GetMissileStationIcon();
    break;
  default:
    iconRes = gpTweakPlayerRes->GetElevatorIcon();
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
  const float x = gpTweakAutoMapper->mDoorCenterC;
  const float y = gpTweakAutoMapper->mDoorCenterB;
  const float z = gpTweakAutoMapper->mDoorCenterA;
  switch (surfaceIdx) {
  case 0:
    return mTransform * CVector3f::Zero();
  case 1:
    return mTransform * CVector3f(0.f, 0.f, 2.f * z);
  case 2:
    return mTransform * CVector3f(0.f, -y, 0.f);
  case 3:
    return mTransform * CVector3f(0.f, y, 0.f);
  case 4:
    return mTransform * CVector3f(-x, 0.f, 0.f);
  case 5:
    return mTransform * CVector3f(x, 0.f, 0.f);
  default:
    return CVector3f::Zero();
  }
}

bool CMappableObject::GetIsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& mwInfo) const {
  bool areaVis = mwInfo.IsAreaVisible(mObjId.AreaNum());
  switch (mVisibilityMode) {
  case kVM_Always:
    return true;
  case kVM_MapStationOrVisit:
  case kVM_MapStationOrVisit2:
    return worldVis || areaVis;
  case kVM_Visit:
    if (IsDoorType(mType)) {
      return mwInfo.IsDoorVisited(mObjId);
    }
    return areaVis;
  case kVM_Never:
    return false;
  default:
    return true;
  }
}

CTransform4f CMappableObject::AdjustTransformForType() const {
  const float doorCenterX = gpTweakAutoMapper->mDoorCenterA;
  const float doorCenterZ = gpTweakAutoMapper->mDoorCenterC;
  if (mType == kMOT_BigDoor1) {
    return (GetTransform() * CTransform4f(CMatrix3f::RotateZ(CRelAngle::FromDegrees(90.f)),
                                          CVector3f(0.f, 0.f, doorCenterX * -1.4f))) *
           CTransform4f::Scale(1.5f);
  } else if (mType == kMOT_BigDoor2) {
    return (GetTransform() *
            CTransform4f(CMatrix3f::RotateZ(CRelAngle::FromDegrees(-90.f)),
                         CVector3f(0.f, doorCenterZ * -2.f, doorCenterX * -1.4f))) *
           CTransform4f::Scale(1.5f);
  } else if (mType == kMOT_IceDoorCeiling || mType == kMOT_WaveDoorCeiling ||
             mType == kMOT_PlasmaDoorCeiling) {
    return GetTransform() * CTransform4f(CMatrix3f::RotateY(CRelAngle::FromDegrees(90.f)),
                                         CVector3f(doorCenterX * -1.65f, 0.f, doorCenterZ * -1.5f));
  } else if (mType == kMOT_IceDoorFloor || mType == kMOT_WaveDoorFloor ||
             mType == kMOT_PlasmaDoorFloor) {
    return GetTransform() * CTransform4f(CMatrix3f::RotateY(CRelAngle::FromDegrees(90.f)),
                                         CVector3f(doorCenterX * -1.65f, 0.f, doorCenterZ * -1.f));
  } else if (mType == kMOT_IceDoorFloor2 || mType == kMOT_WaveDoorFloor2 ||
             mType == kMOT_PlasmaDoorFloor2) {
    return GetTransform() * CTransform4f(CMatrix3f::RotateY(CRelAngle::FromDegrees(90.f)),
                                         CVector3f(doorCenterX * -0.49f, 0.f, doorCenterZ * -1.f));
  } else if (IsDoorType(mType)) {
    return GetTransform();
  }
  return CTransform4f::Translate(GetTransform().GetTranslation());
}
