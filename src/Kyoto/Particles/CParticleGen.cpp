#include "Kyoto/Particles/CParticleGen.hpp"

void CParticleGen::AddModifier(CWarp* warp) {
  mModifiersList.push_back(warp);
}
