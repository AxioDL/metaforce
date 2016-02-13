#include "CParticleGen.hpp"

namespace Retro
{

void CParticleGen::AddModifier(CWarp* mod)
{
    x8_modifierList.push_back(mod);
}

}
