#pragma once

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CTransform.hpp"
#include "IFactory.hpp"
#include "CToken.hpp"
#include "CMapArea.hpp"

namespace urde {
class CStateManager;
class CMapUniverse {
public:
  class CMapUniverseDrawParms {
    float x0_alpha;
    int x4_wldIdx;
    CAssetId x8_wldRes;
    int xc_closestHex;
    float x10_flashPulse;
    // const CStateManager& x14_mgr;
    const zeus::CTransform& x18_model;
    const zeus::CTransform& x1c_view;

  public:
    CMapUniverseDrawParms(float alpha, int wldIdx, CAssetId wldRes, int closestHex, float flashPulse,
                          const CStateManager& mgr, const zeus::CTransform& model, const zeus::CTransform& view)
    : x0_alpha(alpha)
    , x4_wldIdx(wldIdx)
    , x8_wldRes(wldRes)
    , xc_closestHex(closestHex)
    , x10_flashPulse(flashPulse)
    ,
        // x14_mgr(mgr),
        x18_model(model)
    , x1c_view(view) {}
    int GetFocusWorldIndex() const { return x4_wldIdx; }
    const zeus::CTransform& GetCameraTransform() const { return x1c_view; }
    const zeus::CTransform& GetPaneProjectionTransform() const { return x18_model; }
    float GetAlpha() const { return x0_alpha; }
    CAssetId GetWorldAssetId() const { return x8_wldRes; }
    int GetClosestArea() const { return xc_closestHex; }
    float GetFlashPulse() const { return x10_flashPulse; }
  };

  class CMapObjectSortInfo {
    float x0_zDist;
    int x4_wldIdx;
    int x8_hexIdx;
    int xc_surfIdx;
    zeus::CColor x10_surfColor;
    zeus::CColor x14_outlineColor;

  public:
    CMapObjectSortInfo(float zDist, int wldIdx, int hexIdx, int surfIdx, const zeus::CColor& surf,
                       const zeus::CColor& outline)
    : x0_zDist(zDist)
    , x4_wldIdx(wldIdx)
    , x8_hexIdx(hexIdx)
    , xc_surfIdx(surfIdx)
    , x10_surfColor(surf)
    , x14_outlineColor(outline) {}
    const zeus::CColor& GetOutlineColor() const { return x14_outlineColor; }
    const zeus::CColor& GetSurfaceColor() const { return x10_surfColor; }
    int GetObjectIndex() const { return xc_surfIdx; }
    int GetAreaIndex() const { return x8_hexIdx; }
    int GetWorldIndex() const { return x4_wldIdx; }
    float GetZDistance() const { return x0_zDist; }
  };

  class CMapWorldData {
    std::string x0_label;
    CAssetId x10_worldAssetId;
    zeus::CTransform x14_transform;
    std::vector<zeus::CTransform> x44_hexagonXfs;
    zeus::CColor x54_surfColorSelected;
    zeus::CColor x58_outlineColorSelected = zeus::CColor(1.0f, 0.0f, 1.0f);
    zeus::CColor x5c_surfColorUnselected = zeus::CColor(1.0f, 0.0f, 1.0f);
    zeus::CColor x60_outlineColorUnselected = zeus::CColor(1.0f, 0.0f, 1.0f);
    zeus::CVector3f x64_centerPoint = zeus::skZero3f;

  public:
    CMapWorldData(CInputStream& in, u32 version);
    CAssetId GetWorldAssetId() const { return x10_worldAssetId; }
    const zeus::CVector3f& GetWorldCenterPoint() const { return x64_centerPoint; }
    std::string_view GetWorldLabel() const { return x0_label; }
    const zeus::CTransform& GetWorldTransform() const { return x14_transform; }
    const zeus::CTransform& GetMapAreaData(s32 idx) const { return x44_hexagonXfs[idx]; }
    u32 GetNumMapAreaDatas() const { return x44_hexagonXfs.size(); }
    const zeus::CColor& GetOutlineColorUnselected() const { return x60_outlineColorUnselected; }
    const zeus::CColor& GetOutlineColorSelected() const { return x58_outlineColorSelected; }
    const zeus::CColor& GetSurfaceColorUnselected() const { return x5c_surfColorUnselected; }
    const zeus::CColor& GetSurfaceColorSelected() const { return x54_surfColorSelected; }
  };

private:
  CAssetId x0_hexagonId;
  TLockedToken<CMapArea> x4_hexagonToken;
  std::vector<CMapWorldData> x10_worldDatas;
  zeus::CVector3f x20_universeCenter = zeus::skZero3f;
  float x2c_universeRadius = 1600.f;

public:
  CMapUniverse(CInputStream&, u32);
  const CMapWorldData& GetMapWorldData(s32 idx) const { return x10_worldDatas[idx]; }
  const CMapWorldData& GetMapWorldDataByWorldId(CAssetId id) const {
    for (const CMapWorldData& data : x10_worldDatas)
      if (data.GetWorldAssetId() == id)
        return data;
    return x10_worldDatas.front();
  }
  u32 GetNumMapWorldDatas() const { return x10_worldDatas.size(); }
  float GetMapUniverseRadius() const { return x2c_universeRadius; }
  const zeus::CVector3f& GetMapUniverseCenterPoint() const { return x20_universeCenter; }
  void Draw(const CMapUniverseDrawParms&, const zeus::CVector3f&, float, float) const;
  std::vector<CMapWorldData>::const_iterator begin() const { return x10_worldDatas.cbegin(); }
  std::vector<CMapWorldData>::const_iterator end() const { return x10_worldDatas.cend(); }
};

CFactoryFnReturn FMapUniverseFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                     CObjectReference*);

} // namespace urde
