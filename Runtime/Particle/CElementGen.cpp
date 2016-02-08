#include "CElementGen.hpp"
#include "CGenDescription.hpp"

namespace Retro
{

CElementGen::CElementGen(const TToken<CGenDescription>& gen,
                         EModelOrientationType orientType,
                         EOptionalSystemFlags flags)
: x1c_genDesc(gen), x28_orientType(orientType)
{

}

}
