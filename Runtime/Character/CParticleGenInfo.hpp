#pragma once

#include <memory>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CParticleData.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CParticleGen;
class CStateManager;
struct SObjectTag;

enum class EParticleGenType { Normal, Auxiliary };

class CParticleGenInfo {
  SObjectTag x4_part;
  float xc_seconds;
  std::string x10_boneName;
  float x20_curTime = 0.f;
  bool x24_active = false;
  CParticleData::EParentedMode x28_parentMode;
  s32 x2c_flags;
  zeus::CVector3f x30_particleScale;
  float x3c_finishTime = 0.f;
  bool x40_grabInitialData = false;
  zeus::CTransform x44_transform;
  zeus::CVector3f x74_offset;
  EParticleGenType x80_type;

public:
  CParticleGenInfo(const SObjectTag& part, int frameCount, std::string_view boneName, const zeus::CVector3f& scale,
                   CParticleData::EParentedMode parentMode, int flags, EParticleGenType type);

  virtual ~CParticleGenInfo() = default;
  virtual void AddToRenderer() = 0;
  virtual void Render() = 0;
  virtual void Update(float dt, CStateManager& stateMgr) = 0;
  virtual void SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr) = 0;
  virtual void SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr) = 0;
  virtual void SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr) = 0;
  virtual void SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr) = 0;
  virtual void SetGlobalScale(const zeus::CVector3f& scale) = 0;
  virtual void SetParticleEmission(bool isActive, CStateManager& stateMgr) = 0;
  virtual bool IsSystemDeletable() const = 0;
  virtual std::optional<zeus::CAABox> GetBounds() const = 0;
  virtual bool HasActiveParticles() const = 0;
  virtual void DestroyParticles() = 0;
  virtual bool HasLight() const = 0;
  virtual TUniqueId GetLightId() const = 0;
  virtual void DeleteLight(CStateManager& stateMgr) = 0;
  virtual void SetModulationColor(const zeus::CColor& color) = 0;

  void SetFlags(s32 f) { x2c_flags = f; }
  s32 GetFlags() const { return x2c_flags; }
  void SetIsGrabInitialData(bool g) { x40_grabInitialData = g; }
  bool GetIsGrabInitialData() const { return x40_grabInitialData; }
  bool GetIsActive() const { return x24_active; }
  void SetIsActive(bool a) { x24_active = a; }
  void OffsetTime(float dt) { x20_curTime += dt; }
  const zeus::CVector3f& GetCurOffset() const { return x74_offset; }
  void SetCurOffset(const zeus::CVector3f& offset) { x74_offset = offset; }
  const zeus::CTransform& GetCurTransform() const { return x44_transform; }
  void SetCurTransform(const zeus::CTransform& xf) { x44_transform = xf; }
  const zeus::CVector3f& GetCurScale() const { return x30_particleScale; }
  void SetCurScale(const zeus::CVector3f& scale) { x30_particleScale = scale; }
  void SetInactiveStartTime(float s) { xc_seconds = s; }
  float GetInactiveStartTime() const { return xc_seconds; }
  void MarkFinishTime() { x3c_finishTime = x20_curTime; }
  float GetFinishTime() const { return x3c_finishTime; }
  float GetCurrentTime() const { return x20_curTime; }
  void SetCurrentTime(float t) { x20_curTime = t; }
  EParticleGenType GetType() const { return x80_type; }

  CParticleData::EParentedMode GetParentedMode() const { return x28_parentMode; }
  std::string_view GetLocatorName() const { return x10_boneName; }
};

class CParticleGenInfoGeneric : public CParticleGenInfo {
  std::shared_ptr<CParticleGen> x84_system;
  TUniqueId x88_lightId;

public:
  CParticleGenInfoGeneric(const SObjectTag& part, const std::weak_ptr<CParticleGen>& system, int frames,
                          std::string_view boneName, const zeus::CVector3f& scale,
                          CParticleData::EParentedMode parentMode, int flags, CStateManager& stateMgr, TAreaId,
                          int lightId, EParticleGenType state);

  void AddToRenderer() override;
  void Render() override;
  void Update(float dt, CStateManager& stateMgr) override;
  void SetOrientation(const zeus::CTransform& xf, CStateManager& stateMgr) override;
  void SetTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr) override;
  void SetGlobalOrientation(const zeus::CTransform& xf, CStateManager& stateMgr) override;
  void SetGlobalTranslation(const zeus::CVector3f& trans, CStateManager& stateMgr) override;
  void SetGlobalScale(const zeus::CVector3f& scale) override;
  void SetParticleEmission(bool isActive, CStateManager& stateMgr) override;
  bool IsSystemDeletable() const override;
  std::optional<zeus::CAABox> GetBounds() const override;
  bool HasActiveParticles() const override;
  void DestroyParticles() override;
  bool HasLight() const override;
  TUniqueId GetLightId() const override;
  void DeleteLight(CStateManager& mgr) override;
  void SetModulationColor(const zeus::CColor& color) override;
  const std::shared_ptr<CParticleGen> GetParticleSystem() const { return x84_system; }
};
} // namespace urde
