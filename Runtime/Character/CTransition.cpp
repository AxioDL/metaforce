#include "CTransition.hpp"

namespace urde
{

CTransition::CTransition(CInputStream& in)
    : x0_id(in.readUint32Big())
    , x4_animA(in.readUint32Big())
    , x8_animB(in.readUint32Big())
    , xc_trans(CMetaTransFactory::CreateMetaTrans(in))
{
}

}
