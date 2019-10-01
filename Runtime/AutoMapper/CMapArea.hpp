#pragma once

#include <memory>
#include <vector>

#include "Runtime/CResFactory.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/AutoMapper/CMappableObject.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/Shaders/CMapSurfaceShader.hpp"

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CAABox.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class IWorld;
class CMapArea {
public:
  class CMapAreaSurface {
    friend class CMapArea;
    zeus::CVector3f x0_normal;
    zeus::CVector3f xc_centroid;
    const u8* x18_surfOffset;
    const u8* x1c_outlineOffset;
    u32 m_primStart;
    u32 m_primCount;
    struct Instance {
      CMapSurfaceShader m_surfacePrims;
      std::vector<CLineRenderer> m_linePrims;
      Instance(boo::IGraphicsDataFactory::Context& ctx, const boo::ObjToken<boo::IGraphicsBufferS>& vbo,
               const boo::ObjToken<boo::IGraphicsBufferS>& ibo)
      : m_surfacePrims(ctx, vbo, ibo) {}
      Instance(Instance&&) = default;
      Instance& operator=(Instance&&) = default;
    };
    std::vector<Instance> m_instances;

  public:
    CMapAreaSurface(const void* surfBuf);
    CMapAreaSurface(CMapAreaSurface&&) = default;
    void PostConstruct(const u8* buf, std::vector<u32>& index);
    void Draw(const zeus::CVector3f* verts, const zeus::CColor& surfColor, const zeus::CColor& lineColor,
              float lineWidth, int instIdx = 0) const;
    const zeus::CVector3f& GetNormal() const { return x0_normal; }
    const zeus::CVector3f& GetCenterPosition() const { return xc_centroid; }
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
  std::vector<zeus::CVector3f> m_verts;
  u8* x40_surfaceStart;
  std::vector<CMapAreaSurface> m_surfaces;
  std::unique_ptr<u8[]> x44_buf;
  boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
  boo::ObjToken<boo::IGraphicsBufferS> m_ibo;

public:
  CMapArea(CInputStream& in, u32 size);
  void PostConstruct();
  bool GetIsVisibleToAutoMapper(bool worldVis, bool areaVis) const;
  zeus::CVector3f GetAreaCenterPoint() const { return x10_box.center(); }
  const zeus::CAABox& GetBoundingBox() const { return x10_box; }
  const CMappableObject& GetMappableObject(int idx) const { return m_mappableObjects[idx]; }
  const CMapAreaSurface& GetSurface(int idx) const { return m_surfaces[idx]; }
  u32 GetNumMappableObjects() const { return m_mappableObjects.size(); }
  u32 GetNumSurfaces() const { return m_surfaces.size(); }
  zeus::CTransform GetAreaPostTransform(const IWorld& world, TAreaId aid) const;
  static const zeus::CVector3f& GetAreaPostTranslate(const IWorld& world, TAreaId aid);
  const zeus::CVector3f* GetVertices() const { return m_verts.data(); }
};

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&, CObjectReference*);
} // namespace urde
