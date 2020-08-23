#pragma once

namespace urde {
class CParticleGen;

class IWeaponRenderer {
public:
  virtual ~IWeaponRenderer() = default;
  virtual void AddParticleGen(CParticleGen&) = 0;
};

class CDefaultWeaponRenderer : public IWeaponRenderer {
public:
  void AddParticleGen(CParticleGen&) override;
};

} // namespace urde
