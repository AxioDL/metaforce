#include "CParticleGen.hpp"

namespace pshag
{

void CParticleGen::AddModifier(CWarp* mod)
{
    x8_modifierList.push_back(mod);
}

}
