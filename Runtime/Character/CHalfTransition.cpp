#include "CHalfTransition.hpp"
#include "CMetaTransFactory.hpp"

namespace urde
{

CHalfTransition::CHalfTransition(CInputStream& in)
{
    x0_id = in.readUint32Big();
    x4_trans = CMetaTransFactory::CreateMetaTrans(in);
}

}
