#pragma once

#include <array>
#include <list>
#include <memory>
#include <optional>
#include <vector>

#include "Runtime/CRandom16.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Graphics/CLineRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleGen.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CElectricDescription;

class CParticleElectric : public CParticleGen {
  static u16 g_GlobalSeed;

public:
  static void SetGlobalSeed(u16 seed) { g_GlobalSeed = seed; }
  class CLineManager {
    friend class CParticleElectric;
    std::vector<zeus::CVector3f> x0_verts;
    std::array<float, 3> x10_widths = {1.f, 2.f, 3.f};
    std::array<zeus::CColor, 3> x1c_colors;
    zeus::CAABox x28_aabb = zeus::CAABox();
  };

  class CParticleElectricManager {
    friend class CParticleElectric;
    u32 x0_idx;
    u32 x4_slif;
    u32 x8_startFrame;
    u32 xc_endFrame;
    int x10_gpsmIdx = -1;
    int x14_epsmIdx = -1;

  public:
    CParticleElectricManager(u32 idx, u32 slif, u32 startFrame)
    : x0_idx(idx), x4_slif(slif), x8_startFrame(startFrame), xc_endFrame(startFrame + slif) {}
  };

private:
  TLockedToken<CElectricDescription> x1c_elecDesc;
  int x28_currentFrame = 0;
  int x2c_LIFE;
  double x30_curTime = 0.0;
  zeus::CVector3f x38_translation;
  zeus::CTransform x44_orientation;
  zeus::CTransform x74_invOrientation;
  zeus::CVector3f xa4_globalTranslation;
  zeus::CTransform xb0_globalOrientation;
  zeus::CVector3f xe0_globalScale = zeus::skOne3f;
  zeus::CVector3f xec_localScale = zeus::skOne3f;
  zeus::CTransform xf8_cachedXf;
  float x128 = 0.f;
  float x12c = 0.f;
  zeus::CAABox x130_buildBounds = zeus::CAABox();
  CRandom16 x14c_randState;
  int x150_SSEG = 8;
  int x154_SCNT = 1;
  int x158 = 0;
  float x15c_genRem = 0.f;
  zeus::CAABox x160_systemBounds = zeus::CAABox();
  std::optional<zeus::CVector3f> x178_overrideIPos;
  std::optional<zeus::CVector3f> x188_overrideIVel;
  std::optional<zeus::CVector3f> x198_overrideFPos;
  std::optional<zeus::CVector3f> x1a8_overrideFVel;
  zeus::CColor x1b8_moduColor;
  rstl::reserved_vector<bool, 32> x1bc_allocated;
  rstl::reserved_vector<std::unique_ptr<CParticleSwoosh>, 32> x1e0_swooshGenerators;
  rstl::reserved_vector<std::unique_ptr<CLineManager>, 32> x2e4_lineManagers;
  std::list<CParticleElectricManager> x3e8_electricManagers;
  std::vector<std::unique_ptr<CElementGen>> x400_gpsmGenerators;
  std::vector<std::unique_ptr<CElementGen>> x410_epsmGenerators;
  std::vector<zeus::CVector3f> x420_calculatedVerts;
  std::vector<float> x430_fractalMags;
  std::vector<zeus::CVector3f> x440_fractalOffsets;
  bool x450_24_emitting : 1 = true;
  bool x450_25_haveGPSM : 1 = false;
  bool x450_26_haveEPSM : 1 = false;
  bool x450_27_haveSSWH : 1 = false;
  bool x450_28_haveLWD : 1 = false;
  bool x450_29_transformDirty : 1 = true;

  size_t m_nextLineRenderer = 0;
  std::vector<std::unique_ptr<CLineRenderer>> m_lineRenderers;

  void SetupLineGXMaterial();
  void DrawLineStrip(const std::vector<zeus::CVector3f>& verts, float width, const zeus::CColor& color);
  void RenderLines();
  void RenderSwooshes();
  void UpdateCachedTransform();
  void UpdateLine(size_t idx, int frame);
  void UpdateElectricalEffects();
  void CalculateFractal(int start, int end, float ampl, float ampd);
  void CalculatePoints();
  void CreateNewParticles(int count);
  void AddElectricalEffects();
  void BuildBounds();

public:
  explicit CParticleElectric(const TToken<CElectricDescription>& desc);

  bool Update(double) override;
  void Render(const CActorLights* lights = nullptr) override;
  void SetOrientation(const zeus::CTransform& orientation) override;
  void SetTranslation(const zeus::CVector3f& translation) override;
  void SetGlobalOrientation(const zeus::CTransform& orientation) override;
  void SetGlobalTranslation(const zeus::CVector3f& translation) override;
  void SetGlobalScale(const zeus::CVector3f& scale) override;
  void SetLocalScale(const zeus::CVector3f& scale) override;
  void SetParticleEmission(bool emitting) override;
  void SetModulationColor(const zeus::CColor&) override;
  void SetOverrideIPos(const zeus::CVector3f& vec) { x178_overrideIPos.emplace(vec); }
  void SetOverrideIVel(const zeus::CVector3f& vec) { x188_overrideIVel.emplace(vec); }
  void SetOverrideFPos(const zeus::CVector3f& vec) { x198_overrideFPos.emplace(vec); }
  void SetOverrideFVel(const zeus::CVector3f& vec) { x1a8_overrideFVel.emplace(vec); }
  const zeus::CTransform& GetOrientation() const override;
  const zeus::CVector3f& GetTranslation() const override;
  const zeus::CTransform& GetGlobalOrientation() const override;
  const zeus::CVector3f& GetGlobalTranslation() const override;
  const zeus::CVector3f& GetGlobalScale() const override;
  const zeus::CColor& GetModulationColor() const override;
  bool IsSystemDeletable() const override;
  std::optional<zeus::CAABox> GetBounds() const override;
  u32 GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  bool GetParticleEmission() const override;
  void DestroyParticles() override;
  void Reset() override {}
  void ForceParticleCreation(s32 count) {
    CGlobalRandom gRnd{x14c_randState};
    CreateNewParticles(count);
  }
  FourCC Get4CharId() const override { return FOURCC('ELSC'); }
};

} // namespace urde
