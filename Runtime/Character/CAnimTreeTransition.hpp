#ifndef __PSHAG_CANIMTREETRANSITION_HPP__
#define __PSHAG_CANIMTREETRANSITION_HPP__

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

#endif // __PSHAG_CANIMTREETRANSITION_HPP__
