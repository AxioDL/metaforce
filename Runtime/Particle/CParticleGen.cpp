#include "CParticleGen.hpp"

namespace urde
{

void CParticleGen::AddModifier(CWarp* mod)
{
    x4_modifierList.push_back(mod);
}

}
