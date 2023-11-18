#include "Runtime/AutoMapper/CMapUniverse.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/GameGlobalObjects.hpp"

namespace metaforce {

CMapUniverse::CMapUniverse(CInputStream& in, u32 version) : x0_hexagonId(in.Get<CAssetId>()) {
  x4_hexagonToken = g_SimplePool->GetObj({FOURCC('MAPA'), x0_hexagonId});
  u32 count = in.ReadLong();
  x10_worldDatas.reserve(count);
  for (u32 i = 0; i < count; ++i)
    x10_worldDatas.emplace_back(in, version);
}

CMapUniverse::CMapWorldData::CMapWorldData(CInputStream& in, u32 version)
: x0_label(in.Get<std::string>()), x10_worldAssetId(in) {
  x14_transform  = in.Get<zeus::CTransform>();
  const u32 worldCount = in.ReadLong();
  x44_hexagonXfs.reserve(worldCount);
  for (u32 i = 0; i < worldCount; ++i) {
    x44_hexagonXfs.emplace_back() = in.Get<zeus::CTransform>();
  }

  if (version != 0)
    x54_surfColorSelected = in.Get<zeus::CColor>();
  else
    x54_surfColorSelected.fromRGBA32(255 | (u32(x10_worldAssetId.Value()) & 0xFFFFFF00));

  x58_outlineColorSelected = zeus::CColor::lerp(zeus::skWhite, x54_surfColorSelected, 0.5f);
  x5c_surfColorUnselected = zeus::CColor::lerp(zeus::skBlack, x54_surfColorSelected, 0.5f);
  x60_outlineColorUnselected = zeus::CColor::lerp(zeus::skWhite, x5c_surfColorUnselected, 0.5f);

  for (const zeus::CTransform& xf : x44_hexagonXfs)
    x64_centerPoint += xf.origin;

  x64_centerPoint *= zeus::CVector3f(1.0f / float(x44_hexagonXfs.size()));
}

void CMapUniverse::Draw(const CMapUniverseDrawParms& parms, const zeus::CVector3f&, float, float) {
  if (!x4_hexagonToken.IsLoaded()) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CMapUniverse::Draw", zeus::skBlue);

  u32 totalSurfaceCount = 0;
  for (const CMapWorldData& data : x10_worldDatas)
    totalSurfaceCount += data.GetNumMapAreaDatas() * x4_hexagonToken->GetNumSurfaces();

  std::vector<CMapObjectSortInfo> sortInfos;
  sortInfos.reserve(totalSurfaceCount);

  for (size_t w = 0; w < x10_worldDatas.size(); ++w) {
    const CMapWorldData& data = x10_worldDatas[w];
    const CMapWorldInfo& mwInfo = *g_GameState->StateForWorld(data.GetWorldAssetId()).MapWorldInfo();
    if (!mwInfo.IsAnythingSet())
      continue;
    zeus::CColor surfColor, outlineColor;
    if (s32(w) == parms.GetFocusWorldIndex()) {
      surfColor = data.GetSurfaceColorSelected();
      surfColor.a() *= parms.GetAlpha();
      outlineColor = data.GetOutlineColorSelected();
      outlineColor.a() *= parms.GetAlpha();
    } else {
      surfColor = data.GetSurfaceColorUnselected();
      surfColor.a() *= parms.GetAlpha();
      outlineColor = data.GetSurfaceColorUnselected();
      outlineColor.a() *= parms.GetAlpha();
    }

    for (u32 h = 0; h < data.GetNumMapAreaDatas(); ++h) {
      zeus::CTransform hexXf = parms.GetCameraTransform().inverse() * data.GetMapAreaData(h);
      for (u32 s = 0; s < x4_hexagonToken->GetNumSurfaces(); ++s) {
        const CMapArea::CMapAreaSurface& surf = x4_hexagonToken->GetSurface(s);
        zeus::CVector3f centerPos = hexXf * surf.GetCenterPosition();
        sortInfos.emplace_back(centerPos.y(), w, h, s, surfColor, outlineColor);
      }
    }
  }

  std::sort(sortInfos.begin(), sortInfos.end(), [](const CMapObjectSortInfo& a, const CMapObjectSortInfo& b) {
    return a.GetZDistance() > b.GetZDistance();
  });

  int lastWldIdx = -1;
  int lastHexIdx = -1;
  size_t instIdx = 0;
  for (const CMapObjectSortInfo& info : sortInfos) {
    const CMapWorldData& mwData = x10_worldDatas[info.GetWorldIndex()];
    zeus::CColor surfColor = info.GetSurfaceColor();
    zeus::CColor outlineColor = info.GetOutlineColor();
    if (parms.GetWorldAssetId() == mwData.GetWorldAssetId() && parms.GetClosestArea() == info.GetAreaIndex()) {
      surfColor = zeus::CColor::lerp(g_tweakAutoMapper->GetSurfaceSelectVisitedColor(),
                                     g_tweakAutoMapper->GetAreaFlashPulseColor(), parms.GetFlashPulse());
      surfColor.a() = info.GetSurfaceColor().a();
      outlineColor = zeus::CColor::lerp(g_tweakAutoMapper->GetOutlineSelectVisitedColor(),
                                        g_tweakAutoMapper->GetAreaFlashPulseColor(), parms.GetFlashPulse());
      outlineColor.a() = info.GetOutlineColor().a();
    }

    zeus::CTransform hexXf = mwData.GetMapAreaData(info.GetAreaIndex());
    hexXf.orthonormalize();
    CMapArea::CMapAreaSurface& surf = x4_hexagonToken->GetSurface(info.GetObjectIndex());
    zeus::CColor color(std::max(0.f, (-parms.GetCameraTransform().basis[1]).dot(hexXf.rotate(surf.GetNormal()))) *
                           g_tweakAutoMapper->GetMapSurfaceNormColorLinear() +
                       g_tweakAutoMapper->GetMapSurfaceNormColorConstant());
    surfColor *= color;

    if (info.GetAreaIndex() != lastHexIdx || info.GetWorldIndex() != lastWldIdx)
      CGraphics::SetModelMatrix(parms.GetPaneProjectionTransform() * mwData.GetMapAreaData(info.GetAreaIndex()));

    surf.Draw(x4_hexagonToken->GetVertices(), surfColor, outlineColor, 2.f, instIdx++);
  }
}

CFactoryFnReturn FMapUniverseFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&, CObjectReference*) {
  in.ReadLong();
  u32 version = in.ReadLong();

  return TToken<CMapUniverse>::GetIObjObjectFor(std::make_unique<CMapUniverse>(in, version));
}

} // namespace metaforce
