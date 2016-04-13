#ifndef __URDE_CMETATRANSFACTORY_HPP__
#define __URDE_CMETATRANSFACTORY_HPP__

#include "IOStreams.hpp"
#include "IMetaTrans.hpp"

namespace urde
{

class CMetaTransFactory
{
public:
    static std::shared_ptr<IMetaTrans> CreateMetaTrans(CInputStream& in);
};

}

#endif // __URDE_CMETATRANSFACTORY_HPP__
