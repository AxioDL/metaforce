#ifndef __URDE_CANIMTREETRANSITION_HPP__
#define __URDE_CANIMTREETRANSITION_HPP__

#include "RetroTypes.hpp"
#include "CAnimTreeTweenBase.hpp"

namespace urde
{

class CAnimTreeTransition : public CAnimTreeTweenBase
{
public:
    static std::string CreatePrimitiveName();
};

}

#endif // __URDE_CANIMTREETRANSITION_HPP__
