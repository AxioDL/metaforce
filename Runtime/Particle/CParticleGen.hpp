#pragma once

#include <list>
#include <optional>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CLight.hpp"

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CWarp;
class CActorLights;

struct CParticle {
  int x0_endFrame = 0;
  zeus::CVector3f x4_pos;
  zeus::CVector3f x10_prevPos;
  zeus::CVector3f x1c_vel;
  int x28_startFrame = 0;
  float x2c_lineLengthOrSize = 0.f;
  float x30_lineWidthOrRota = 0.f;
  zeus::CColor x34_color = {1.f, 0.f, 1.f, 1.f};
};

class CParticleGen {
protected:
  std::list<CWarp*> x4_modifierList;

public:
  virtual ~CParticleGen() = default;

  virtual bool Update(double) = 0;
  virtual void Render() = 0;
  virtual void SetOrientation(const zeus::CTransform&) = 0;
  virtual void SetTranslation(const zeus::CVector3f&) = 0;
  virtual void SetGlobalOrientation(const zeus::CTransform&) = 0;
  virtual void SetGlobalTranslation(const zeus::CVector3f&) = 0;
  virtual void SetGlobalScale(const zeus::CVector3f&) = 0;
  virtual void SetLocalScale(const zeus::CVector3f&) = 0;
  virtual void SetParticleEmission(bool) = 0;
  virtual void SetModulationColor(const zeus::CColor&) = 0;
  virtual void SetGeneratorRate(float rate) {}
  virtual const zeus::CTransform& GetOrientation() const = 0;
  virtual const zeus::CVector3f& GetTranslation() const = 0;
  virtual const zeus::CTransform& GetGlobalOrientation() const = 0;
  virtual const zeus::CVector3f& GetGlobalTranslation() const = 0;
  virtual const zeus::CVector3f& GetGlobalScale() const = 0;
  virtual const zeus::CColor& GetModulationColor() const = 0;
  virtual float GetGeneratorRate() const { return 1.f; }
  virtual bool IsSystemDeletable() const = 0;
  virtual std::optional<zeus::CAABox> GetBounds() const = 0;
  virtual u32 GetParticleCount() const = 0;
  virtual bool SystemHasLight() const = 0;
  virtual CLight GetLight() const = 0;
  virtual bool GetParticleEmission() const = 0;
  virtual void DestroyParticles() = 0;
  virtual void Reset() = 0;
  virtual FourCC Get4CharId() const = 0;

  virtual void AddModifier(CWarp* mod);
};

} // namespace metaforce
