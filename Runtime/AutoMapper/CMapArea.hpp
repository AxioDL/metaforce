#pragma once

#include <memory>
#include <vector>

#include "Runtime/Graphics/CCubeModel.hpp"
#include "Runtime/AutoMapper/CMappableObject.hpp"
#include "Runtime/CResFactory.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
using CColor = zeus::CColor;
using CVector3f = zeus::CVector3f;

class IWorld;
class CMapArea {
public:
  class CMapAreaSurface {
    friend class CMapArea;
    CVector3f x0_normal;
    CVector3f xc_centroid;
    const u32* x18_surfOffset;
    const u32* x1c_outlineOffset;

  public:
    explicit CMapAreaSurface(const void* surfBuf);

    void PostConstruct(const void* buf);
    void Draw(TConstVectorRef verts, const CColor& surfColor, const CColor& lineColor, float lineWidth) const;

    static void SetupGXMaterial();

    const CVector3f& GetNormal() const { return x0_normal; }
    const CVector3f& GetCenterPosition() const { return xc_centroid; }
  };
  enum class EVisMode { Always, MapStationOrVisit, Visit, Never };

private:
  u32 x0_magic;
  u32 x4_version;
  u32 x8_;
  EVisMode xc_visibilityMode;
  zeus::CAABox x10_box;
  u32 x28_mappableObjCount;
  u32 x2c_vertexCount;
  u32 x30_surfaceCount;
  u32 x34_size;
  u8* x38_moStart;
  std::vector<CMappableObject> m_mappableObjects;
  u8* x3c_vertexStart;
  std::vector<aurora::Vec3<float>> m_verts;
  u8* x40_surfaceStart;
  std::vector<CMapAreaSurface> m_surfaces;
  std::unique_ptr<u8[]> x44_buf;

public:
  explicit CMapArea(CInputStream& in, u32 size);
  void PostConstruct();
  bool GetIsVisibleToAutoMapper(bool worldVis, bool areaVis) const;
  zeus::CVector3f GetAreaCenterPoint() const { return x10_box.center(); }
  const zeus::CAABox& GetBoundingBox() const { return x10_box; }
  CMappableObject& GetMappableObject(int idx) { return m_mappableObjects[idx]; }
  const CMappableObject& GetMappableObject(int idx) const { return m_mappableObjects[idx]; }
  CMapAreaSurface& GetSurface(int idx) { return m_surfaces[idx]; }
  const CMapAreaSurface& GetSurface(int idx) const { return m_surfaces[idx]; }
  u32 GetNumMappableObjects() const { return m_mappableObjects.size(); }
  u32 GetNumSurfaces() const { return m_surfaces.size(); }
  zeus::CTransform GetAreaPostTransform(const IWorld& world, TAreaId aid) const;
  static const zeus::CVector3f& GetAreaPostTranslate(const IWorld& world, TAreaId aid);
  TConstVectorRef GetVertices() const { return m_verts; }
};

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&, CObjectReference*);
} // namespace metaforce
