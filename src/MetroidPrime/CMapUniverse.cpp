#include "MetroidPrime/CMapUniverse.hpp"

#include "Kyoto/CVParamTransfer.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CMapArea.hpp"
#include "MetroidPrime/CMapWorldInfo.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldState.hpp"
#include "MetroidPrime/Tweaks/CTweakAutoMapper.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"
#include "rstl/rc_ptr.hpp"

class CMapObjectSortInfoGreaterThan {
public:
  bool operator()(const CMapUniverse::CMapObjectSortInfo& a,
                  const CMapUniverse::CMapObjectSortInfo& b) const {
    return a.GetZDistance() > b.GetZDistance();
  }
};

CMapUniverse::CMapObjectSortInfo::CMapObjectSortInfo(float zDistance, int worldIndex, int areaIndex,
                                                     int objectIndex, CColor surfaceColor,
                                                     CColor outlineColor)
: mZDistance(zDistance)
, mWorldIndex(worldIndex)
, mAreaIndex(areaIndex)
, mObjectIndex(objectIndex)
, mSurfaceColor(surfaceColor)
, mOutlineColor(outlineColor) {}

CMapUniverse::CMapUniverseDrawParms::CMapUniverseDrawParms(float alpha, int wldIdx, CAssetId wldRes,
                                                           int closestHex, float flashPulse,
                                                           const CStateManager& mgr,
                                                           const CTransform4f& model,
                                                           const CTransform4f& view)
: mAlpha(alpha)
, mFocusWorldIndex(wldIdx)
, mFocusWorldRes(wldRes)
, mFocusAreaIndex(closestHex)
, mFlashPulse(flashPulse)
, mStateManager(mgr)
, mPaneProjectionTransform(model)
, mCameraTransform(view) {}

CMapUniverse::CMapUniverse(CInputStream& in, uint version)
: x0_hexagonId(in.Get< CAssetId >())
, x4_hexagonToken(gpSimplePool->GetObj(SObjectTag('MAPA', x0_hexagonId)))
, x20_universeCenter(CVector3f::Zero())
, x2c_universeRadius(1600.f) {
  x10_worldDatas.reserve(in.Get< uint >());
  for (int i = 0; i < x10_worldDatas.capacity(); ++i) {
    x10_worldDatas.push_back(CMapWorldData(in, version));
  }
  x4_hexagonToken.Lock();
}

CMapUniverse::~CMapUniverse() {}

void CMapUniverse::Draw(const CMapUniverseDrawParms& parms, const CVector3f& pos, float depth1,
                        float depth2) const {
  if (x4_hexagonToken.TryCache()) {
    int surfaceCount = 0;
    int numSurfaces = x4_hexagonToken.GetObject()->GetNumSurfaces();
    for (int i = 0; i < x10_worldDatas.size(); ++i) {
      int numAreas = x10_worldDatas[i].GetNumMapAreaDatas();
      surfaceCount += numSurfaces * numAreas;
    }
    rstl::vector< CMapObjectSortInfo > sortInfos;
    sortInfos.reserve(surfaceCount);
    const float alpha = parms.GetAlpha();
    const CTransform4f& model = parms.GetPaneProjectionTransform();
    const CTransform4f& camera = parms.GetCameraTransform();
    const CMapArea& mapArea = *x4_hexagonToken.GetObject();
    for (int w = 0; w < x10_worldDatas.size(); ++w) {
      const CMapWorldData& world = x10_worldDatas[w];
      if (gpGameState->StateForWorld(world.GetWorldAssetId()).GetMapWorldInfo()->IsAnythingSet()) {
        const bool selected = w == parms.GetFocusWorldIndex();
        const CColor& surfaceColor =
            selected ? world.GetSurfaceColorSelected().WithAlphaModulatedBy(alpha)
                     : world.GetSurfaceColorUnselected().WithAlphaModulatedBy(alpha);
        const CColor& outlineColor =
            selected ? world.GetOutlineColorSelected().WithAlphaModulatedBy(alpha)
                     : world.GetSurfaceColorUnselected().WithAlphaModulatedBy(alpha);
        for (int h = 0; h < world.GetNumMapAreaDatas(); ++h) {
          const CTransform4f transform =
              camera.GetQuickInverse() * world.GetMapAreaData(h).GetTransform();
          for (int s = 0; s < mapArea.GetNumSurfaces(); ++s) {
            const CVector3f surfaceCenter = mapArea.GetSurface(s).GetCenterPosition();
            const CVector3f center = transform * surfaceCenter;
            sortInfos.push_back(
                CMapObjectSortInfo(center.GetY(), w, h, s, surfaceColor, outlineColor));
          }
        }
      }
    }
    if (sortInfos.size() > 0) {
      CMapObjectSortInfoGreaterThan comparator;
      rstl::sort(sortInfos.begin(), sortInfos.end(), comparator);
      CMapArea::CMapAreaSurface::SetupGXMaterial();
      int lastWorld = -1;
      int lastArea = -1;
      for (int i = 0; i < sortInfos.size(); ++i) {
        int worldIndex = sortInfos[i].GetWorldIndex();
        int areaIndex = sortInfos[i].GetAreaIndex();
        int objectIndex = sortInfos[i].GetObjectIndex();
        CColor surfaceColor = sortInfos[i].GetSurfaceColor();
        CColor outlineColor = sortInfos[i].GetOutlineColor();
        const CMapWorldData& world = x10_worldDatas[worldIndex];
        const CTransform4f& transform = world.GetMapAreaData(areaIndex).GetTransform();
        const CMapArea::CMapAreaSurface& surface = mapArea.GetSurface(objectIndex);
        if (parms.GetFocusWorldRes() == world.GetWorldAssetId() &&
            areaIndex == parms.GetFocusAreaIndex()) {
          const uchar surfaceAlpha = surfaceColor.GetAlphau8();
          const uchar outlineAlpha = outlineColor.GetAlphau8();
          surfaceColor =
              CColor::Lerp(gpTweakAutoMapper->x4c_surfaceSelectColorVisited,
                           gpTweakAutoMapper->xf4_areaFlashPulseColor, parms.GetFlashPulse());
          outlineColor =
              CColor::Lerp(gpTweakAutoMapper->x50_outlineSelectColorVisited,
                           gpTweakAutoMapper->xf4_areaFlashPulseColor, parms.GetFlashPulse());
          surfaceColor.SetAlpha(surfaceAlpha);
          outlineColor.SetAlpha(outlineAlpha);
        }
        CTransform4f normalTransform = transform;
        normalTransform.Orthonormalize();
        const float linear = gpTweakAutoMapper->x54_mapSurfaceNormColorLinear;
        const float constant = gpTweakAutoMapper->x58_mapSurfaceNormColorConstant;
        const float lit =
            linear *
            rstl::max_val(0.f, CVector3f::Dot(-1.f * camera.GetForward(),
                                              normalTransform.Rotate(surface.GetNormal())));
        const float shade = constant + lit;
        const CColor shadingColor(shade, shade, shade, 1.f);
        const CColor color = CColor::Modulate(surfaceColor, shadingColor);
        const bool changed = lastArea != areaIndex || lastWorld != worldIndex;
        if (changed) {
          gpRender->SetModelMatrix(model * transform);
        }
        surface.Draw(changed ? mapArea.GetVertices() : TMapVertices(), color, outlineColor, 2.f);
        lastWorld = worldIndex;
        lastArea = areaIndex;
      }
    }
  }
}

CMapUniverse::CMapAreaData::CMapAreaData(CInputStream& in) : mTransform(in) {}

CMapUniverse::CMapWorldData::CMapWorldData(CInputStream& in, uint version)
: x0_label(in)
, x10_worldAssetId(in.Get< CAssetId >())
, x14_transform(in)
, x44_areaDatas(in)
, x54_surfColorSelected(version != 0 ? CColor(in) : CColor(0))
, x58_outlineColorSelected(static_cast< uchar >(255), 0, 255)
, x5c_surfColorUnselected(static_cast< uchar >(255), 0, 255)
, x60_outlineColorUnselected(static_cast< uchar >(255), 0, 255)
, x64_centerPoint(CVector3f::Zero()) {
  if (version == 0) {
    x54_surfColorSelected = CColor(x10_worldAssetId).WithAlphaOf(1.f);
  }

  x58_outlineColorSelected = CColor::Lerp(CColor::White(), x54_surfColorSelected, 0.5f);
  x5c_surfColorUnselected = CColor::Lerp(CColor::Black(), x54_surfColorSelected, 0.5f);
  x60_outlineColorUnselected = CColor::Lerp(CColor::White(), x5c_surfColorUnselected, 0.5f);

  for (int i = 0; i < x44_areaDatas.size(); ++i) {
    x64_centerPoint += x44_areaDatas[i].GetTransform().GetTranslation();
  }
  x64_centerPoint *= 1.f / static_cast< float >(x44_areaDatas.size());
}

const CMapUniverse::CMapWorldData& CMapUniverse::GetMapWorldDataByWorldId(CAssetId id) {
  for (int i = 0; i < GetNumMapWorldDatas(); i++) {
    const CMapWorldData& wd = GetMapWorldData(i);
    if (wd.GetWorldAssetId() == id) {
      return wd;
    }
  }

  return x10_worldDatas[0];
}
const CFactoryFnReturn FMapUniverseFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer) {
  in.Get< uint >();
  uint version = in.Get< uint >();
  return rs_new CMapUniverse(in, version);
}
