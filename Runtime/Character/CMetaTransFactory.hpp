#pragma once

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

