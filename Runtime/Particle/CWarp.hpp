#pragma once

#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Particle/CParticleGen.hpp"

namespace urde {

class CWarp {
public:
  virtual ~CWarp() = default;
  virtual bool UpdateWarp() = 0;
  virtual void ModifyParticles(std::vector<CParticle>& particles) = 0;
  virtual void Activate(bool) = 0;
  virtual bool IsActivated() = 0;
  virtual FourCC Get4CharID() = 0;
};

} // namespace urde
