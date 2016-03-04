#include "CParticleGen.hpp"

namespace urde
{

void CParticleGen::AddModifier(CWarp* mod)
{
    x8_modifierList.push_back(mod);
}

}
