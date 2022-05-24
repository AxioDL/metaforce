#pragma once

#include <array>
#include <optional>
#include <utility>

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Graphics/Shaders/CMapSurfaceShader.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>

namespace metaforce {
class CMapWorldInfo;
class CStateManager;

class CMappableObject {
public:
  enum class EMappableObjectType {
    BlueDoor = 0,
    ShieldDoor = 1,
    IceDoor = 2,
    WaveDoor = 3,
    PlasmaDoor = 4,
    BigDoor1 = 5,
    BigDoor2 = 6,
    IceDoorCeiling = 7,
    IceDoorFloor = 8,
    WaveDoorCeiling = 9,
    WaveDoorFloor = 10,
    PlasmaDoorCeiling = 11,
    PlasmaDoorFloor = 12,
    IceDoorFloor2 = 13,
    WaveDoorFloor2 = 14,
    PlasmaDoorFloor2 = 15,
    DownArrowYellow = 27, /* Maintenance Tunnel */
    UpArrowYellow = 28,   /* Phazon Processing Center */
    DownArrowGreen = 29,  /* Elevator A */
    UpArrowGreen = 30,    /* Elite Control Access */
    DownArrowRed = 31,    /* Elevator B */
    UpArrowRed = 32,      /* Fungal Hall Access */
    TransportLift = 33,
    SaveStation = 34,
    MissileStation = 37
  };

  enum class EVisMode { Always, MapStationOrVisit, Visit, Never, MapStationOrVisit2 };

private:
  static std::array<zeus::CVector3f, 8> skDoorVerts;
  static std::array<u16, 24> skDoorIndices;

  EMappableObjectType x0_type;
  EVisMode x4_visibilityMode;
  TEditorId x8_objId;
  u32 xc_;
  zeus::CTransform x10_transform;

  struct DoorSurface {
    CMapSurfaceShader m_surface;
    CLineRenderer m_outline;
    explicit DoorSurface()
    : m_surface(skDoorVerts, skDoorIndices)
    , m_outline(CLineRenderer::EPrimitiveMode::LineLoop, 5, {}, false, false, true) {}
  };
  std::optional<DoorSurface> m_doorSurface;

  zeus::CTransform AdjustTransformForType() const;
  std::pair<zeus::CColor, zeus::CColor> GetDoorColors(int idx, const CMapWorldInfo& mwInfo, float alpha) const;

public:
  explicit CMappableObject(const void* buf);
  CMappableObject(CMappableObject&&) = default;
  void PostConstruct(const void*);
  const zeus::CTransform& GetTransform() const { return x10_transform; }
  EMappableObjectType GetType() const { return x0_type; }
  void Draw(int, const CMapWorldInfo&, float, bool);
  void DrawDoorSurface(int curArea, const CMapWorldInfo& mwInfo, float alpha, int surfIdx, bool needsVtxLoad);
  zeus::CVector3f BuildSurfaceCenterPoint(int surfIdx) const;
  bool IsDoorConnectedToArea(int idx, const CStateManager&) const;
  bool IsDoorConnectedToVisitedArea(const CStateManager&) const;
  bool IsVisibleToAutoMapper(bool worldVis, const CMapWorldInfo& mwInfo) const;
  bool GetIsSeen() const;
  void CreateDoorSurface() { m_doorSurface.emplace(); }

  static void ReadAutoMapperTweaks(const ITweakAutoMapper&);
  static bool GetTweakIsMapVisibilityCheat();
  static bool IsDoorType(EMappableObjectType type) {
    return type >= EMappableObjectType::BlueDoor && type <= EMappableObjectType::PlasmaDoorFloor2;
  }
  static void Shutdown();
};
} // namespace metaforce
